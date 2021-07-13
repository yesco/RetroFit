// TODO: make streamed

const http = require('http');
const h2t = require('./html.js');

let log = 0;


//let url = 'http://www.google.com';
let url = '';

url = 'http://yesco.org/'; // TODO: 404
// TODO: write redirector? or just call wget?

url = 'http://runeberg.org/';
url = 'http://yesco.org/index.html';
url = 'http://www.lysator.liu.se/';

url = process.argv[2] || url;


function wget(url, cb) {
  let chunks = [];

  http.get(url, (res)=>{
    res.on('data', (data)=>{
      // TODO: this assumes UTF-8
      // can't recover iso-8895-1
      chunks.push(data.toString());
      if (log) console.log("CHUNK:", data.toString());
    });
    res.on('end', ()=>{
      if (log) console.log("END:"); 
      cb(null, chunks.join(''));
    });
  });
}

let debug = 1;

function teletext2ansi(t) {
  // - \u0320 non-breakable space
  // - \u030a force newline
  // (These retain color after new line)
  // - \u0102 set green START
  // - \u0202 set green END (matching)
  // - \u0112 set green back START
  // - \u0212 set green back END (matching)

  let stack = [];
  let bg = '\u0117'; // white text
  let fg = '\u0100'; // black back
  let normal = '\u0110\u0102'; // green on black
  let bold = '[1m';
  let nobold = '[22m';

  function ord(ch) {
    return ch.charCodeAt(0);
  }
  function color(ch) {
    let n = ord(ch);
    let c = n & 0xf;
    // TODO: these were intially made to occupy one space on screen ala oric attributes...
    //return `<0x${n.toString(16)}>`;
if (1) 
    if (c >= 0 && c <= 8) {
      // x = `<0x${n.toString(16)}>`;
      if ((n & 0x10) == 0x00) {
        fg = ch;
        return (debug?`[fg:${c}] `:(c&&c!=7?bold:nobold)+'[3'+c+'m');
        return (debug?`[fg:${c}] `:bold+'[3'+c+'m');
      }
      if ((n & 0x10) == 0x10) {
        bg = ch;
        return (debug?`[bg:${c}] `:'[4'+c+'m');
      }
    }
    return `<?: 0x${n.toString(16)}>`;
  }

  function start(ch) {
    stack.push([ch, bg, fg]);
    return color(ch);
  }

  function end(ch) {
    let [_ch,_bg,_fg] = stack.pop();
    // TODO: make sure _ch matches ch
    // TODO: only if changed
    if (debug) {
      return ' R:'+color(_bg)+' R:'+color(_fg);
    } else {
      return color(_bg)+color(_fg);
    }
    bg = _bg; fg = _fg;
  }

  function startend(c) {
    return c.replace(/([\u0100-\u0107\u0110-\u0117])/g, start)
      .replace(/([\u0200-\u0207\u0210-\u0217])/g, end);
  }

  return (bg+fg+"\n"+t+normal+'\n')
    .replace(/([\u0100-\u0217])/g, startend)
    // collapse white space
    .replace(/ +/g, ' ')
    // at this stage \n newlines do matter
    // TODO: However, they are intermixed with
    // ansi control-codes.

    .replace(/\n+/g, "\n")
    .replace(/ *\n* *\u0320/g, ' ')
    // keep hard spaces

    .replace(/\u0320/g, (debug?'_':' ')) // hard-space
    .replace(/[\n ]*\u030a[ \n]*/g, (debug?"\\N\n":"\n")) // hard-newline

    // \n+ maybe not good for <pre>
    .replace(/\n+/g, '[K\n') // workaround eol-bug! https://stackoverflow.com/questions/53740460/ansi-escape-code-weird-behavior-at-end-of-line
  ;
}

wget(url, (status, h)=>{
  if (log) console.log("WGET: f", h);
  let nid = 0;

  let hclean = h
    .replace(/([\u0100-\u0107\u0110-\u0117])/g, '<?>')
    .replace(/([\u0200-\u0207\u0210-\u0217])/g, '<?>');

  let tele = h2t.HTML2TELETEXT(nid, hclean);
  if (log) {
    process.stdout.write("\n\n---TELE---\n\n");
    process.stdout.write(tele);
    process.stdout.write("\n");
  }

  if (log) {
    debug = 1;
    let dansi = teletext2ansi(tele);
    process.stdout.write("\n\n---debugANSI---\n\n");
    process.stdout.write(dansi);
    process.stdout.write("\n\n");
  }

  debug = 0;
  let ansi = teletext2ansi(tele);
  let fg = 3, bg = 0;
//  process.stdout.write('[H[2J[3J'); // cls

//  process.stdout.write('[1m[3'+4+'m');
  //process.stdout.write('[1m[3'+1+'m');
  //process.stdout.write('[4'+3+'m');
//  process.stdout.write('./retro ');

  process.stdout.write('[3'+bg+'m');
  process.stdout.write('[4'+fg+'m');
  //process.stdout.write('./retro ');
  process.stdout.write('<retro> ');

  process.stdout.write('[3'+fg+'m');
  process.stdout.write('[4'+bg+'m');
  if (log) {
    process.stdout.write("\n\n---ANSI---\n\n\n\n");
  }
  process.stdout.write(' '+url+'\n');

  process.stdout.write(ansi);
  process.stdout.write("\n\n");
});
