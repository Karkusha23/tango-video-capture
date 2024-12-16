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

  var deviceParamEl = document.getElementById('params')

  document.getElementById('post_params').addEventListener('click', function() 
  { 
    var newDeviceParams = {
      rulerStartX: Number(document.getElementById('ruler_start_x').value) ?? 0,
      rulerStartY: Number(document.getElementById('ruler_start_y').value) ?? 0,
      rulerEndX: Number(document.getElementById('ruler_end_x').value) ?? 0,
      rulerEndY: Number(document.getElementById('ruler_end_y').value) ?? 0,
      rulerLength: Number(document.getElementById('ruler_length').value) ?? 0,
      threshold: Number(document.getElementById('threshold').value) ?? 25
    }
    httpPostRequest(deviceParamsUrl, JSON.stringify(newDeviceParams)); 
  })

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
        deviceParamEl.innerHTML = httpGetRequest(deviceParamsUrl);
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