// 引入web3
var Web3 = require('web3');
var config=require('../web3lib/config');
var web3sync = require('../web3lib/web3sync');
//const Tx = require('ethereumjs-tx').Transaction;
if (typeof web3 !== 'undefined') {
    web3 = new Web3(web3.currentProvider);
} else {
  web3 = new Web3(new Web3.providers.HttpProvider(config.HttpProvider));
}


// 账号
var currentAccount = "0x9c2b3b96f9db9ccf95d37d5ba4f009fd8e111b2d";

var receiverAccount = "0xa540f5da47e38de6718ca94895856409b529b2a1";



async function getbalance(address) { 
	// 查询以太币余额
	var result = web3.eth.getBalance(address);
	console.log(result);
	console.log(result.toString(10)); // '1000000000000'
	console.log(result.toNumber()); // 1000000000000
}

async function transfer(_from,_to,_value) { 
	var block = await web3sync.getBlockNumber();
	console.log('block number:' + block);
        result = await web3sync.transfer(currentAccount,'9cf7ae0c5dc725525875fc665217a99f7f9c0cfe68e702207a4afa973d16bf27','0xc23cba39584c83c7d3d218051382b28b5a562b75',2000000000);
        console.log(result);
/*
	var privateKey = new Buffer.from('9cf7ae0c5dc725525875fc665217a99f7f9c0cfe68e702207a4afa973d16bf27', 'hex')
	var rawTx = {
         from: '0x9c2b3b96f9db9ccf95d37d5ba4f009fd8e111b2d',
	 nonce: '0x00',
	 gasPrice: '0x09184e72a000', 
	 gasLimit: '0x2710',
	 to: '0xc23cba39584c83c7d3d218051382b28b5a562b75', 
	 value: '2000000000000000',
	}
	var tx = new Tx(rawTx);
	tx.sign(privateKey);
	var serializedTx = tx.serialize();
	//console.log(serializedTx.toString('hex'));
        var result =  web3.eth.sendRawTransaction('0x' + serializedTx.toString('hex'));
	
	console.log(result);
*/
}

async function newaccount(privateKey) { 
	var result = web3.eth.accounts.privateKeyToAccount(privateKey);
	console.log(result);
}


async function main(args){
	
	var func=args[2];
	switch(func) {
	case 'getbalance':
		if ( args.length < 4 )
		{
			console.log("parameters error \n");
			process.exit(1);
			
		}
		getbalance(args[3]);
		break;
	case 'newaccount':
		if ( args.length < 4 )
		{
			console.log("parameters error \n");
			process.exit(1);
		}
		newaccount(args[3]);
		break;
	case 'transfer':
		if ( args.length < 6 )
		{
			console.log("parameters error \n");
			process.exit(1);
		}
		transfer(args[3],args[4],args[5]);
		break;
	default:
		console.log("unknown command");
		process.exit(1);
	}
	
}

console.log();
var options = process.argv;
if( options.length < 3 )
{
    console.log('Usage: node balance.js func');
    process.exit(1);
}

main(options);

