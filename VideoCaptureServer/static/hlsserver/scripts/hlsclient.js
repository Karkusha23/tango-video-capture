function httpGetRequest(url)
{
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open("GET", url, false);
  xmlHttp.send(null);
  return xmlHttp.responseText;
}

function httpPostRequest(url, body)
{
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.open("POST", url, false);
  xmlHttp.send(body)
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
  var deviceParamsUrl = '/device/' + deviceName + '/params';
  var sourceUrl = '/media_no_cache/playlists/' + deviceName.replaceAll('/', '') + '/playlist.m3u8';
  var sourceExistsUrl = '/media_exists/playlists/' + deviceName.replaceAll('/', '') + '/playlist.m3u8';
  var readyToLoad = false

  var newdeviceparams = { rulerEndX: 100, rulerEndY: 100, rulerLength: 100, threshold: 25 };

  document.getElementById('post_params').addEventListener('click', function() { httpPostRequest(deviceParamsUrl, JSON.stringify(newdeviceparams)); })

  hls.on(Hls.Events.error, function(event, data)
  {
    hlsError = true;
    console.log('Hls error');
  })

  hls.on(Hls.Events.MANIFEST_PARSED, function() 
  {
    video.play();
    console.log('event');
  });

  const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
  (async () => {
    while (true)
      {
        await sleep(2000);
        httpPostRequest(deviceHeartbeatUrl, '');
        var device_params = JSON.parse(httpGetRequest(deviceParamsUrl));
        console.log(device_params);
        console.log('Heartbeat');
      }
    })();

  const sleep2 = ms => new Promise(resolve => setTimeout(resolve, ms));
  (async () => {
    while (true)
    {
      var res = httpGetRequest(sourceExistsUrl);
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