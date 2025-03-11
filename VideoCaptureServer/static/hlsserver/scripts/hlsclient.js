class HLSClient
{
  // Synchronous HTTP GET REQUEST
  static httpGetRequest(url)
  {
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open("GET", url, false);
    xmlHttp.send(null);
    return xmlHttp.responseText;
  }

  // Synchronous HTTP POST REQUEST
  static httpPostRequest(url, body)
  {
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.open("POST", url, false);
    xmlHttp.send(body);
    return xmlHttp.responseText;
  }

  static createVideoObj(id)
  {
    var videoEl = document.getElementById(id);
    var result = {
      el: videoEl,
      width: Number(videoEl.getAttribute('width')), 
      height: Number(videoEl.getAttribute('height')),
    };
    result.ratio = result.width / result.height;
    return result;
  }

  static createDeviceUrls(dataID)
  {
    var deviceName = document.getElementById(dataID).getAttribute('device');
    var result = {
      heartbeat: '/device/' + deviceName + '/heartbeat',
      params: '/device/' + deviceName + '/params',
      source: '/media_no_cache/playlists/' + deviceName.replaceAll('/', '') + '/playlist.m3u8',
      sourceExists: '/media_exists/playlists/' + deviceName.replaceAll('/', '') + '/playlist.m3u8'
    };
    return result;
  }

  static getVideoClickXY(video, clientX, clientY, isFullscreen)
  {
    var rect = video.el.getBoundingClientRect();
    var scaleFullscreen = 1;
    var scaleX = 1;
    var scaleY = 1;
    var borderLeft = 0;
    var borderTop = 0;
    var fullscreenMarginLeft = 0;
    var fullscreenMarginTop = 0;
    if (isFullscreen)
    {
      var fullscreenRatio = window.innerWidth / window.innerHeight;
      if (video.ratio > fullscreenRatio)
      {
        scaleFullscreen = window.innerWidth / video.width;
        fullscreenMarginTop = (window.innerHeight - video.height * scaleFullscreen) / 2;
      }
      else if (video.ratio < fullscreenRatio)
      {
        scaleFullscreen = window.innerHeight / video.height
        fullscreenMarginLeft = (window.innerWidth - video.width * scaleFullscreen) / 2;
      }
    }
    else
    {
      const styles = window.getComputedStyle(video.el);
      scaleX = parseFloat(styles.transform.split(',')[0].replace('matrix(', '')) || 1;
      scaleY = parseFloat(styles.transform.split(',')[3]) || 1;
      borderLeft = parseInt(styles.borderLeftWidth, 10) || 0;
      borderTop = parseInt(styles.borderTopWidth, 10) || 0;
    }
    var x = (clientX - rect.left - borderLeft - fullscreenMarginLeft) / (scaleX * scaleFullscreen);
    var y = (clientY - rect.top - borderTop - fullscreenMarginTop) / (scaleY * scaleFullscreen);
    return {x: x, y: y};
  }
}