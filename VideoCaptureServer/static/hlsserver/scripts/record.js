if(Hls.isSupported())
{
    var video = HLSClient.createVideoObj('video');
    var deviceUrls = HLSClient.createDeviceUrls('data');
    var info = document.getElementById('info');

    document.getElementById('title').innerHTML = 'Record ' + deviceUrls.record;
    document.getElementById('h1').innerHTML = 'Record ' + deviceUrls.record;

    var hls = new Hls();

    hls.loadSource(deviceUrls.source);
    hls.attachMedia(video.el);

    var infoReceiver = new VideoInfoReceiver(deviceUrls.recordDataHeader);

    (async () => {
        while (true)
        {
            info.innerHTML = JSON.stringify(infoReceiver.getInfo(video.el.currentTime));
            await new Promise(r => setTimeout(r, 1000));
        }
    })();
}
else
{
    console.log('HLS is not supported!')
}