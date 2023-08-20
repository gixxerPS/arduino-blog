'use strict';
const mysql = require('mysql');
const util = require('util');
const udp = require('dgram');

const buffer = require('buffer');
const server = udp.createSocket('udp4');

const UDP_PORT = 8266;

const MYSQL_HOST = 'localhost';
const MYSQL_USER = 'node';
const MYSQL_PASS = '12345678';
const MYSQL_DB = 'nodedb';
const MYSQL_TABLE = 'bme280';

var con = mysql.createConnection({
  host: MYSQL_HOST,
  user: MYSQL_USER,
  password: MYSQL_PASS,
  database: MYSQL_DB,
  port: '/var/run/mysqld/mysqld.sock'
});

// dieser event wird ausgelöst wenn eine 'message' eintrifft
server.on('message', function (msg, info) {
  //console.log(`RX from ${info.address}:${info.port} ${msg.toString()}`);

  // Beispiel Nachricht: 'Messstation 1;Temperatur=26.3;Feuchte=82;Druck=1008;'
  // Beispiel Nachricht: 'Messstation 1;26.3;82;1008;' ohne ueberschrift wuerde auch klappen
  var cols = msg.toString().split(';');
  var temp  = parseFloat(cols[1].split('=')[1] || cols[1] || 0.0);
  var rf    = parseFloat(cols[2].split('=')[1] || cols[2] || 0.0);
  var press = parseFloat(cols[3].split('=')[1] || cols[3] || 0.0);
  
  //console.log(`temp = ${temp}°C; rf = ${rf}; press = ${press}`);
  var myquery = `INSERT INTO ${MYSQL_TABLE} (timestamp, temp, rf, press) VALUES (${parseInt(Date.now() / 1000)}, ${temp}, ${rf}, ${press})`;
  console.log(`query=${myquery}`);
  con.query(myquery, (err, res) => {
    if (err) {
      return console.error(`query error: ${err.toString()}`);
    } else {
      console.log(util.inspect(res));
    }
  });
});

//emits when socket is ready and listening for datagram msgs
server.on('listening',function(){
  var address = server.address();
  var port = address.port;
  var family = address.family;
  var ipaddr = address.address;
  console.log('Server is listening at port' + port);
  console.log('Server ip :' + ipaddr);
  console.log('Server is IP4/IP6 : ' + family);
});

server.bind(UDP_PORT); // UDP server starten