import * as fs from 'fs';
const ffmpeg = require('fluent-ffmpeg');

ffmpeg.setFfmpegPath('../ffmpeg/ffmpeg-master-latest-win64-gpl/bin/ffmpeg.exe');
export let jpeg_buf: Buffer[] = []
export let bufReady: boolean = false;
export let lastPacketTime: number;

export function handleBuffer(msg: Buffer){
    jpeg_buf.push(msg);
    lastPacketTime = +new Date()
    bufReady = true
}
export function cleanTempFiles(len: number){
    for (let i = 0; i < len; i++) {
        let path = `../tmp/input_${i}.jpg`;
        fs.unlinkSync(path);
    }
    jpeg_buf = []
}

export function checkTimeout() {
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