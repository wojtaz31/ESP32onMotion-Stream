import {handleBuffer} from "./buffer_utils";
import * as datagram from 'dgram';
const socket = datagram.createSocket("udp4");
const port = 6000
export function udpConfig(){
    socket.bind(port);
    console.log('Server is listening at port  ' + port );
    socket.on("message",(msg,receInfo)=>{
        handleBuffer(msg);
    });
}

