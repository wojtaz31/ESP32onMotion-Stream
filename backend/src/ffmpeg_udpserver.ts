import * as datagram from 'dgram';
import {cleanTempFiles, checkTimeout, handleBuffer, jpeg_buf, bufReady} from "./buffer_utils";

const socket = datagram.createSocket("udp4");
const port = 6000
socket.bind(port);
console.log('Server is listening at port  ' + port );

socket.on("message",(msg,receInfo)=>{
    handleBuffer(msg);
});

setInterval(async () => {
    await Promise.resolve();
    if (!bufReady) cleanTempFiles(jpeg_buf.length);
    checkTimeout();
}, 500);