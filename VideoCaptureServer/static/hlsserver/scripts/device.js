if(Hls.isSupported()) 
{
    var video = HLSClient.createVideoObj('video');
    var deviceUrls = HLSClient.createDeviceUrls('data');
    var deviceParams = {};
    var deviceParamsEl = document.getElementById('params');
    var hls = new Hls();
    
    var readyToLoad = false;
    var isFirstFragLoaded = false;
    var isFullscreen = false;
    
    video.el.addEventListener('fullscreenchange', function()
    {
        isFullscreen = document.fullscreenElement === video.el || document.webkitFullscreenElement === video.el;
    });
    
    video.el.addEventListener('click', function(e)
    {
        var point = HLSClient.getVideoClickXY(video, e.clientX, e.clientY, isFullscreen);
        console.log(point.x, point.y);
    });
    
    hls.on(Hls.Events.error, function(event, data)
    {
        hlsError = true;
        console.log('Hls error');
    });
    
    hls.on(Hls.Events.FRAG_LOADED, function() 
    {
        if (!isFirstFragLoaded)
        {
            video.play()
            firstFragLoaded = true
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
          await new Promise(r => setTimeout(r, 2000));
        }
    
        hls.loadSource(deviceUrls.source);
        hls.attachMedia(video.el);
    })();
    
    (async () => {
        while (true)
        {
            await new Promise(r => setTimeout(r, 2000));
            deviceParams = JSON.parse(HLSClient.httpGetRequest(deviceUrls.params))
            deviceParamsEl.innerHTML = JSON.stringify(deviceParams);
            console.log('Heartbeat');
        }
    })();
}
else
{
    console.log('Hls is not supported!')
}