const udp = require('dgram');
const client = udp.createSocket('udp4');

const REMOTE_PORT = 8266;
//const REMOTE_HOST = 'localhost';
const REMOTE_HOST = '192.168.2.64';

// const msg = 'Messstation 1;26.3;82;1008;';
const msg = 'Messstation 1;Temperatur=26.3;Feuchte=82;Druck=1008;';

client.send(Buffer.from(msg), REMOTE_PORT, REMOTE_HOST, (error) => {
  if (error) {
    client.close();
  } else {
    console.log(`TX -> ${REMOTE_HOST}:${REMOTE_PORT}: ${msg}`);
    client.close();
  }
});
