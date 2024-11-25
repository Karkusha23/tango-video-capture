if(Hls.isSupported()) 
{
    console.log('123');
    var video = document.getElementById('video');
    var data = document.getElementById('data');
    var hls = new Hls();
    hls.loadSource('/media_no_cache/playlists/' + data.getAttribute('device') + '/playlist.m3u8');
    hls.attachMedia(video);
    hls.on(Hls.Events.MANIFEST_PARSED,function() 
    {
       video.play();
       console.log('event');
    });

    const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
    (async () => {
      while (true)
      {
        await sleep(2000);
        var xmlHttp = new XMLHttpRequest();
        xmlHttp.open("GET", '/device_heartbeat/' + data.getAttribute('device'), false); // false for synchronous request
        xmlHttp.send(null);
        console.log('Heartbeat');
      }
    })();
}
else
{
    console.log('Hls is not supported!')
}