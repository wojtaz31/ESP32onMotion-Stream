import {cleanTempFiles, checkTimeout, jpeg_buf, bufReady} from "./buffer_utils";
import {udpConfig} from "./udpconfig";

udpConfig();
setInterval(async () => {
    await Promise.resolve();
    if (!bufReady) cleanTempFiles(jpeg_buf.length);
    checkTimeout();
}, 500);