function httpRequest(url)
{
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open("GET", url, false); // false for synchronous request
  xmlHttp.send(null);
  return xmlHttp.responseText;
}

if(Hls.isSupported()) 
{
  console.log('123');
  var video = document.getElementById('video');
  var data = document.getElementById('data');
  var hls = new Hls();
  var deviceName = data.getAttribute('device')
  var deviceHeartbeatUrl = '/device/' + deviceName + '/heartbeat';
  var sourceUrl = '/media_no_cache/playlists/' + deviceName.replaceAll('/', '') + '/playlist.m3u8';
  var sourceExistsUrl = '/media_exists/playlists/' + deviceName.replaceAll('/', '') + '/playlist.m3u8';
  var readyToLoad = false

  hls.on(Hls.Events.error, function(event, data)
  {
    hlsError = true;
    console.log('Hls error');
  })

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
        httpRequest(deviceHeartbeatUrl);
        console.log('Heartbeat');
      }
    })();

  const sleep2 = ms => new Promise(resolve => setTimeout(resolve, ms));
  (async () => {
    while (true)
    {
      var res = httpRequest(sourceExistsUrl);
      if (res == '1')
      {
        break;
      }
      await sleep2(1000);
    }

    hls.loadSource(sourceUrl);
    hls.attachMedia(video);
  })();
}
else
{
    console.log('Hls is not supported!')
}