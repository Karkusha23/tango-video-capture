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
  var video = document.getElementById('video');
  var data = document.getElementById('data');
  var hls = new Hls();
  var deviceName = data.getAttribute('device')
  var deviceHeartbeatUrl = '/device/' + deviceName + '/heartbeat';
  var deviceParamsUrl = '/device/' + deviceName + '/params';
  var sourceUrl = '/media_no_cache/playlists/' + deviceName.replaceAll('/', '') + '/playlist.m3u8';
  var sourceExistsUrl = '/media_exists/playlists/' + deviceName.replaceAll('/', '') + '/playlist.m3u8';
  var readyToLoad = false;
  var isFirstFragLoaded = false;
  var deviceParams = {};
  var deviceParamsEl = document.getElementById('params');

  var isFullscreen = false

  video.addEventListener('fullscreenchange', function()
  {
    isFullscreen = document.fullscreenElement === video || document.webkitFullscreenElement === video;
  })

  video.addEventListener('click', function(e)
  {
    var rect = e.target.getBoundingClientRect();
    const styles = window.getComputedStyle(video);
    var scaleX = 1;
    var scaleY = 1;
    var borderLeft = 0;
    var borderTop = 0;
    var fullscreenMarginLeft = 0;
    var fullscreenMarginTop = 0;
    if (isFullscreen)
    {
      var videoRatio = video.width / video.height;
      var fullscreenRatio = window.innerWidth / window.innerHeight;

      if (videoRatio > fullscreenRatio)
      {
        var scale = window.innerWidth / Number(video.getAttribute('width'))
        fullscreenMarginTop = (window.innerHeight - Number(video.getAttribute('height')) * scale) / 2;
      }
      else if (videoRatio < fullscreenRatio)
      {
        var scale = window.innerHeight / Number(video.getAttribute('height'))
        fullscreenMarginLeft = (window.innerWidth - Number(video.getAttribute('width')) * scale) / 2;
      }
    }
    else
    {
      var scaleX = parseFloat(styles.transform.split(',')[0].replace('matrix(', '')) || 1;
      var scaleY = parseFloat(styles.transform.split(',')[3]) || 1;
      var borderLeft = parseInt(styles.borderLeftWidth, 10) || 0;
      var borderTop = parseInt(styles.borderTopWidth, 10) || 0;
    }
    var x = (e.clientX - rect.left - borderLeft - fullscreenMarginLeft) / scaleX;
    var y = (e.clientY - rect.top - borderTop - fullscreenMarginTop) / scaleY;
    console.log(x + ' ' + y)
  })

  /*document.getElementById('post_params').addEventListener('click', function() 
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
  })*/

  hls.on(Hls.Events.error, function(event, data)
  {
    hlsError = true;
    console.log('Hls error');
  })

  hls.on(Hls.Events.FRAG_LOADED, function() 
  {
    if (!isFirstFragLoaded)
    {
      video.play()
      firstFragLoaded = true
    }
  });

  const sleep = ms => new Promise(resolve => setTimeout(resolve, ms));
  (async () => {
    while (true)
      {
        await sleep(2000);
        var paramsStr = httpGetRequest(deviceParamsUrl)
        deviceParams = JSON.parse(paramsStr)
        deviceParamsEl.innerHTML = JSON.stringify(deviceParams);
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