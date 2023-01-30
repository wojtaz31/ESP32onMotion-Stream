import * as datagram from 'dgram';
import * as fs from 'fs';
let ffmpeg = require('fluent-ffmpeg');

ffmpeg.setFfmpegPath('../ffmpeg/ffmpeg-master-latest-win64-gpl/bin/ffmpeg.exe');
const socket = datagram.createSocket("udp4");
const port = 6000
let jpeg_buf: Buffer[] = []
let bufReady: boolean = false;
let lastPacketTime: number;

function cleanTempFiles(len: number){
    for (let i = 0; i < len; i++) {
        let path = `../tmp/input_${i}.jpg`;
        fs.unlinkSync(path);
    }
    jpeg_buf = []
}
function checkTimeout() {
    if ((+new Date() - lastPacketTime > 1000) && bufReady) {
        for (let i = 0; i < jpeg_buf.length; i++) {
            let path = `../tmp/input_${i}.jpg`;
            fs.writeFileSync(path, jpeg_buf[i]);
        }

        const today = new Date();
        const date = `${today.getFullYear()}-${today.getMonth()}-${today.getDate()}-${today.getHours()}_${today.getMinutes()}_${today.getSeconds()}`;

        let command = ffmpeg();
        command.input('../tmp/input_%d.jpg')
            .output(`../recordings/${date}.avi`)
            .on('start', function(commandLine: any) { console.log('Spawned Ffmpeg with command: ' + commandLine);})
            .run();
        bufReady = false
    }
}

socket.bind(port);
console.log('Server is listening at port  ' + port );

socket.on("message",(msg,receInfo)=>{
    console.log(msg)
    jpeg_buf.push(msg);
    lastPacketTime = +new Date()
    bufReady = true
});

setInterval(async () => {
    await Promise.resolve();
    if (!bufReady) cleanTempFiles(jpeg_buf.length);
    checkTimeout();
}, 500);

