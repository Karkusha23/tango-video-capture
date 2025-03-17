if(Hls.isSupported())
{
    var video = HLSClient.createVideoObj('video');
    var deviceUrls = HLSClient.createDeviceUrls('data');
    var camWidth = Number(document.getElementById('data').getAttribute('camwidth'));
    var camHeight = Number(document.getElementById('data').getAttribute('camheight'));
    var deviceParams = null;
    var deviceParamsEl = document.getElementById('params');
    var hls = new Hls();

    document.getElementById('title').innerHTML = deviceUrls.name;
    document.getElementById('h1').innerHTML = deviceUrls.name;
    
    var readyToLoad = false;
    var isFirstFragLoaded = false;
    var isFullscreen = false;
    var clickPoint = null;

    function readDeviceParams()
    {
        deviceParams = JSON.parse(HLSClient.httpGetRequest(deviceUrls.params))
        deviceParamsEl.innerHTML = JSON.stringify(deviceParams);
        console.log('Heartbeat');
    }
    
    video.el.addEventListener('fullscreenchange', function()
    {
        isFullscreen = document.fullscreenElement === video.el || document.webkitFullscreenElement === video.el;
    });
    
    video.el.addEventListener('click', function(e)
    {
        if (deviceParams == null)
        {
            return;
        }
        point = HLSClient.getVideoClickXY(video, e.clientX, e.clientY, isFullscreen);
        if (point.x >= 0 && point.x <= camWidth && point.y >= 0 && point.y <= camHeight)
        {
            clickPoint = point;
        }
    });
    
    hls.on(Hls.Events.error, function(event, data)
    {
        hlsError = true;
        console.log('Hls error');
    });
    
    (async () => {
        while (true)
        {
          var res = HLSClient.httpGetRequest(deviceUrls.sourceExists);
          console.log(res);
          if (res == '1')
          {
            break;
          }
          await new Promise(r => setTimeout(r, 500));
        }
    
        hls.loadSource(deviceUrls.source);
        hls.attachMedia(video.el);

        while (true)
        {
            if (navigator.userActivation.hasBeenActive)
            {
                break;
            }
            await new Promise(r => setTimeout(r, 500));
        }
        video.el.currentTime = video.el.duration - 3 > 0 ? video.el.duration - 3 : 0;
        video.el.play();
    })();
    
    (async () => {
        while (true)
        {
            await new Promise(r => setTimeout(r, 1000));
            HLSClient.httpPostRequest(deviceUrls.heartbeat, '');
            readDeviceParams();
            if (clickPoint != null)
            {
                if (HLSClient.distance(clickPoint, deviceParams.ruler.start) < HLSClient.distance(clickPoint, deviceParams.ruler.end))
                {
                    deviceParams.ruler.start = clickPoint;
                }
                else
                {
                    deviceParams.ruler.end = clickPoint;
                } 
                clickPoint = null;
                HLSClient.httpPostRequest(deviceUrls.params, JSON.stringify(deviceParams));
                readDeviceParams();
            }
        }
    })();
}
else
{
    console.log('Hls is not supported!')
}