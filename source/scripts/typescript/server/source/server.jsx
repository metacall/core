'use strict';
/* eslint no-console: "off" */

import React from 'react';
import ReactDomServer from 'react-dom/server';
import App from './components/App.jsx';

function index() {
	return (
		<html>
			<head>
				<title>React SSR Example</title>
				<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=0" />
			</head>
			<body>
				<div id="app">${ReactDomServer.renderToString(<App />)}</div>
				<script src="bundle.js"></script>
			</body>
		</html>
	);
}

export default index;
