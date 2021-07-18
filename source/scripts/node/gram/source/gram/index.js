const { StringSession } = require('telegram/sessions');
const { TelegramClient } = require('telegram');

(async function () {
	const string = new StringSession('');
	const client = new TelegramClient(string, 1, '1', {});
	client.connect().then(async () => {
		console.log(`Connected: ${await client.connected}`);
	}).then(() => {
		setTimeout(async () => {
			console.log('Disconnect');
			await client.disconnect();
		}, 1000);
	});
})();

