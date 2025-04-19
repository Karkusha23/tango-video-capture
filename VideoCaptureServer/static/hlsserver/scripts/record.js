if(Hls.isSupported())
{
    var video = HLSClient.createVideoObj('video');
    var deviceUrls = HLSClient.createDeviceUrls('data');

    document.getElementById('title').innerHTML = 'Record ' + deviceUrls.record;
    document.getElementById('h1').innerHTML = 'Record ' + deviceUrls.record;

    var hls = new Hls();

    hls.loadSource(deviceUrls.source);
    hls.attachMedia(video.el);
}
else
{
    console.log('HLS is not supported!')
}