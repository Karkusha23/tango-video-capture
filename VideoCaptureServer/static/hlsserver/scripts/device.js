if(Hls.isSupported())
{
    var video = HLSClient.createVideoObj('video');
    var deviceUrls = HLSClient.createDeviceUrls('data');
    var camWidth = Number(document.getElementById('data').getAttribute('camwidth'));
    var camHeight = Number(document.getElementById('data').getAttribute('camheight'));
    var deviceParams = null;
    var deviceParamsEl = document.getElementById('params');
    var hls = new Hls();

    var recbutton = document.getElementById('recbutton');
    var recname = null;
    console.log(recbutton);

    document.getElementById('title').innerHTML = deviceUrls.name;
    document.getElementById('h1').innerHTML = deviceUrls.name;
    
    var readyToLoad = false;
    var isFirstFragLoaded = false;
    var isFullscreen = false;
    var clickPoint = null;
    var manifestParsed = false;

    hls.on(Hls.Events.MANIFEST_PARSED, function()
    {
        manifestParsed = true;
    });

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

    recbutton.addEventListener('click', function(e)
    {
        if (!recname)
        {
            recname = HLSClient.httpPostRequest(deviceUrls.startrec, '');
            recbutton.innerHTML = "Stop record";
            console.log('Start rec ' + recname);
        }
        else
        {
            window.location.replace('/record/' + recname + '/stoprec');
        }
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

        while (!(navigator.userActivation.hasBeenActive && manifestParsed))
        {
            await new Promise(r => setTimeout(r, 500));
        }
        await new Promise(r => setTimeout(r, 2000));
        
        video.el.currentTime = video.el.duration - 5 > 0 ? video.el.duration - 5 : 0;
        video.el.play();
    })();
    
    (async () => {
        while (true)
        {
            await new Promise(r => setTimeout(r, 1000));
            HLSClient.httpPostRequest(deviceUrls.heartbeat, '');
            if (recname)
            {
                HLSClient.httpPostRequest('/encoder/' + recname + '/heartbeat');
            }
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