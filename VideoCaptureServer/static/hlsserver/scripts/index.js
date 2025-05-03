var recordList = JSON.parse(HLSClient.httpGetRequest('/recordlist'));
var listString = '';
for (var i = 0; i < recordList.length; ++i)
{
    console.log(recordList[i]);
    listString += '<li><a href="/record/' + recordList[i] + '">' + recordList[i] + '</a></li>';
}
document.getElementById('list').innerHTML = listString;
