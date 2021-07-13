// wget.url["pageid linkid"] = url
// wget.url["pageid linktxt"] = url
// wget.pageid = pageid // last seen  
function wget(url, cb) {
  // plain on JSONP! (works from file://)
  let s = document.createElement('script');
  // create a callback that cleans up
  wget.cb = wget.cb || {};
  wget.url = wget.url || {};
  let n = wget.n = (wget.n || 0) + 1;
  wget.url[n] = url;
  wget.pageid = n;
  wget.form[n] = {};
  wget.cb[n] = function(data){
    try {
      cb(data, undefined, n, url);
    } catch(e) {
      alert(`wget.cb: ${url}\n${''+e}\n${''+e.stack}`);
    }
    // cleanup
    s.remove();
    delete wget.cb[n];
  };

  // hardcode some chached files for debug
  if (url == 'R.C') {
    s.src = 'runeberg.org3';
  } else if (url == 'H.C') {
    s.src = 'html-tables.js';
  } else if (url == 'D.C') {
    s.src = 'duckduckgo.js';
  } else if (url == 'G.C') {
    s.src = 'google.com.js';
  } else {
    s.src = 'http://yesco.org/wget.pl?jsonp=(wget.cb['+n+'])&'+url;
  }

  console.log(s.src);

  // TODO: cleanup on error
  s.onerror = 'alert("load err");';
  s.onload  = 'alert("load ok");';
  // actrivate
  document.body.appendChild(s);
}
wget.url = {};
wget.db = {}; // what? cache?
wget.form = {};


