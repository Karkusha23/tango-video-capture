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
    var data = document.getElementById(dataID);
    var deviceName = data.getAttribute('device');
    var encoderName = data.getAttribute('encodername');
    var record = data.getAttribute('record');
    return {
      name: deviceName,
      encoder: encoderName,
      record: record,
      heartbeat: encoderName ? '/encoder/' + encoderName + '/heartbeat' : null,
      params: '/device/' + deviceName + '/params',
      source: '/media_no_cache/playlists/' + (encoderName ? encoderName : 'records/' + record) + '/playlist.m3u8',
      sourceExists: '/media_exists/playlists/' + (encoderName ? encoderName : 'records/' + record) + '/playlist.m3u8',
      startrec: deviceName ? '/device/' + deviceName + '/startrec' : null,
      recordDataHeader: record ? '/media_no_cache/playlists/records/' + record + '/ciheader.header' : null
    };
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
    return {x: Math.round(x), y: Math.round(y)};
  }

  static distance(first, second)
  {
    return Math.sqrt(Math.pow(first.x - second.x, 2) + Math.pow(first.y - second.y, 2))
  }
}

class VideoInfoReceiver
{
  #headerUrl;
  #fragmentInfo;
  #fragments;

  constructor(headerUrl)
  {
    this.#headerUrl = headerUrl;
    this.#fragmentInfo = [];
    this.#fragments = [];
    var headerStr = HLSClient.httpGetRequest(this.#headerUrl).split('\n');
    for (var i = 0; i < headerStr.length; ++i)
    {
      if (headerStr[i] == '')
      {
        continue;
      }
      var info = headerStr[i].split(' ');
      this.#fragmentInfo.push({ url: info[0], pts: Number(info[1]) });
      this.#fragments.push(null);
    }
  }

  getInfo(videoPlayTime)
  {
    var pts = videoPlayTime * 1000;
    console.log(pts);
    var index;
    for (index = 0; index < this.#fragmentInfo.length - 1; ++index)
    {
      if (pts >= this.#fragmentInfo[index].pts && pts < this.#fragmentInfo[index + 1].pts)
      {
        break;
      }
    }
    if (!this.#fragments[index])
    {
      this.#fragments[index] = JSON.parse(HLSClient.httpGetRequest(this.#fragmentInfo[index].url));
    }
    var frag = this.#fragments[index]
    for (var i = 0; i < frag.length - 1; ++i)
    {
      if (pts >= frag[i].pts && pts < frag[i + 1].pts)
      {
        return frag[i].infos;
      }
    }
    return frag[frag.length - 1].infos;
  }
}