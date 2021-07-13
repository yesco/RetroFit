// usage:
// let [a,b,c] = rget('foobar', /(o*)x(|y)/);
// --> ['oo', 'y', undefined]
// no match -> all undefined
function rget(str, re) {
  let m = str.match(re);
  if (!m) return [];
  m.shift();
  return m;
}

function getAttr(s, name) {
  // quoted
  let [q1, val] = rget(
    s, RegExp(`${name}\\s*=\\s*(['"])(.*?)\\1`));
  // try unquoted
  if (!q1) {
    [val] = rget(
      s, RegExp(`${name}\\s*=\\s*([^\\s>]+)`, 'i'));
  }
  return val?val:'';
}

// a11y refers to Accessability!
// - https://www.a11yproject.com/
// try to get the most text descriptive tag
// from attributes
// => '' not important element, don't show!
// => 'some text'
function getA11y(s) {
  let t;

  // <caption...  | <figaction...
  t = s.match(/<caption[\s\S]*?>(.*)<\/caption>/);
  if (t && t[1]) return t[1];
  t = s.match(/<figcaption[\s\S]*?>(.*)<\/figcaption>/);
  if (t && t[1]) return t[1];

  t = getAttr(s, 'aria-label');
  if (t) return t;

  // TODO: this may be on any sub element...
  t = getAttr(s, 'title') || getAttr(s, 'alt');
  if (t) return t;

  // "hidden"
  let [directattr] = rget(s, /^([^\>]*)>/);
  if (directattr && getAttr(directattr, 'aria-hidden'))
    return '';

  // fallback - show name or id tag!
  t = getAttr(s, 'name') || getAttr(s, 'id');

  // actually, give up!
  return '';

  // we don't give up easy, use filename!
  t = getAttr(s, 'src') || getAttr(s, 'href');
  if (!t) return ''; // ok, give UP!

  // foo/fish OR foo/fish/ => 'fish'
  let src = t;
  [t] = rget(t, /([\w\-]+[^\/]*)\/{0,1}$/);
  return t ? t : '';
}

// convert HTML to Teletext
// nid is a "page id" for accessing local links from wget database
// h is the raw HTML
//
// OUTPUT: ANSI-color stream
//
function HTML2TELETEXT(nid, h) {
  // TODO: base URL
  // TODO: move styling to stylemap
  // TODO: handle inline font color
  // TODO: handle style inline color
  // TODO: handle style-sheet and class
  
  // - \u0320 non-breakable space
  // - \u030a force newline
  // (These retain color after new line)
  // - \u0102 set green START
  // - \u0202 set green END (matching)
  // - \u0112 set green back START
  // - \u0212 set green back END (matching)

  // TODO: replace inside out, innermost <>

  // short number for linmks/images
  let n = 0; 

  // TODO: https://www.gsp.com/cgi-bin/man.cgi?section=1&topic=uni2ascii
  // - http://www.ibiblio.org/pub/packages/ccic/software/unix/convert/utf7/
  // - https://gitlab.freedesktop.org/poppler/poppler/blob/poppler-0.75.0/poppler/UTF.cc#L433  (a PDF renderer?)
  // - https://github.com/kmike/text-unidecode
  // - https://metacpan.org/pod/Text::Unidecode
  // - https://github.com/avian2/unidecode
  // - https://www.npmjs.org/package/unidecode
  // - https://github.com/FGRibreau/node-unidecode
  // a11y or aria acceccible,names
  // - https://www.kirupa.com/html5/emoji.htm
  // <p role="img" aria-label="hamburger">🍔</p>
  // - https://github.com/iamcal/emoji-data
  // - https://github.com/bengarrett/RetroTxt
  // !!!!

  // - http://html.com/tables MENU (hide)
  // <span class='mega-toggle-label' role='button' aria-expanded='false'>
  //   role='button' => show small/toggle view?
  //   aria-expanded='false'

  // tables:
  // - could just show an outline
  //   i.e., indent but without tags!
  // - or, render as text table, lynx is bad
  
  // font-size:
  // - style='font-size:'
  // - <big>/<small> (BIG notin HTML5,but small!)
  // - <font size=3> (1..7, +1,-1, notin HTML5!)
  
  // colors
  // - <font color='red'     (not in HTML5!)
  // - <xxx style='color:xxx;'
  // <li class="popular_posts_bars_li popular_posts_bars_color_orange">

  h = h
    .replace(/&raquo;/ig, '&gt;&gt;')
    .replace(/&copy;/ig, '\`')
    .replace(/&#x(\S+);/ig,
	     (a,h)=>String.fromCharCode(parseInt(h, 16)))
	     
    .replace(/&#(\d+);/ig,
	     (a,d)=>String.fromCharCode(+d))

    .replace(/\r/g, '\n')
    .replace(/[\x00-\x09\x11-\x1F]/g, '')
    .replace(/\xa0/g, '\u0320' /*nbsp*/)
    .replace(
      /([^\0-\x7f])/g,
      (a,c)=>`\\u${c.charCodeAt(0).toString(16).padStart(4, '0')}`)

    .replace(/<(\w+)[\s\S]*?aria-hidden=.*?[s\S]*?\/\1>/ig, '')

    .replace(/<!--[\s\S]*?-->/g, '')
    .replace(/<title[\s\S]*?\/title>/g, '')
    .replace(/<style[\s\S]*?\/style>/ig, '')
    .replace(/<script[\s\S]*?\/script>/ig, '')
    .replace(/<head[\s\S]*?\/head>/ig, '')

    .replace(
      /<img[\s\S]*?>/g,
      (a)=>{
	// TODO: render alt as "big font"?

	let alt = getA11y(a);
	// if no useful description give up
	if (!alt) return ''; 

	let src = getAttr(a, 'src');

	if (!alt)
	  alt = getA11y(a);

        return '[img:TODO]';

	let have = wget.url[`${nid} ${src}`];
	if (have && !alt)
	  return `\u0116${have} \u0216`;

	n++;
	let lid = `${nid} ${n}`;
	wget.url[lid] = src;
	wget.url[`${nid} ${src}`] = n;

	return img2teletext(a, src, alt, lid, n);
      })

    .replace(/<br[\s\S]*?>/g, '\u030a')
    .replace(/<hr[\s\S]*?>/g, '\u030a--------------------\u030a')

    .replace(/<b>([\s\S]*?)<\/b>/ig, '\u0101$1\u0201')
    .replace(/<i>([\s\S]*?)<\/i>/ig, '\u0105$1\u0205')
    .replace(/<em>([\s\S]*?)<\/em>/ig, '\u0107\u0110$1&nbsp;\u0207\u0200')

    .replace(
      /\s*<a [\s\S]*?>\s*([\s\S]*?)\s*<\/a>\s*/ig,
      (a, txt)=>{
	// https://news.ycombinator.com/ -> links errononious1

	// TODO: runeberg.org <a><i>xx</i></a> gets to be magnenta - cannot be recogn as link.... :-(
          //<li>Henrik Berg, <a href="/bhspansk/" ><i>Spanska sjukan och dess botande enligt den fysikaliska läkemetoden</i></a> (1918)
	// TODO: relative links... 
	// /foo.bar
	// foo.bar
	// ../foo.bar

	let href = getAttr(a, 'href');
	// save as '1 2'
	// and     '1 TXT'

	// if no text, maybe it's css... (duck..)
	// make simple link
	if (txt == '') 
	  txt = getA11y(a);

	n++;

// TODO:
//	wget.url[`${nid} ${txt}`] = href;
//	let lid = `${nid} ${n}`;
//	wget.url[lid] = href;
	return '\u0104' + txt + '\u0204';


      })
    .replace(/\u0204(\s+)\u0104/g, '\u0304$1')

    .replace(/<iframe[\s\S]*?>/g, (a)=>{
      let src = getAttr(a, 'src');
      if (!src) return;
      
      // TODO: generalize and use for others, mayhbe can use title, alt //
     
      let [txt] = rget(src, /\/([\w\-\.]+)[^\/]*$/);

      n++;
      txt = 'IFRAME:' + (txt?txt:n);

// TODO:
      // TODO: similar to <a ...
//      wget.url[`${nid} ${txt}`] = src;
//      let lid = `${nid} ${n}`;
//      wget.url[lid] = src;
      return '\u0104' + txt + '\u0204';
    })

    .replace(/<option.*?\/option>/ig, '')
    .replace(/<select.*?\/select>/ig, '')
	     
    .replace(/<button[\s\S]*?>([\s\S]*?)<\/button>/g, '[? $1 ]')
    .replace(
      /<input[\s\S]*?>/g,
      (a)=>{
	let name = getAttr(a, 'name') || 'isindex';
	let type = (getAttr(a, 'type') || 'text').toLowerCase();

	let value = getAttr(a, 'value');
	// store values (even hidden)
	if (name && value) {
// TODO:
//	  wget.form[wget.pageid][name] = value;
	}
	if (type === 'hidden') return '';

	if (type === 'submit') {
	  // TODO: ink dissapared!
	  //return `&nbsp;\u0104\u0116${value}\u0216\u0204&nbsp;foo\u030a`;
	  let txt = getA11y(a);
	  let disp = txt || value || 'SUBMIT';
return `&nbsp;&nbsp;\u0103\u0114${disp} \u0203\u0214`;
	  return `
\u0104&nbsp;`+ // TODO: this is so it doesn't take column 01..
// maybe this can be used as difference for block and inline-block??? haha
`\u0113
${disp}
&nbsp;
\u0204
&nbsp;` + // TODO: this is needed between INK+PAPER, why? otherwise cannot restore paper!
`\u0213` + // TODO: 6_1foo (one too many _) 
``;
	} else {
	  let size = getAttr(a, 'size') || 12;
	  // TODO: make sure not to read!
	  value = value.padEnd(size, '\u007E');

	  if (type === 'text') {

	    return `&nbsp;\u0101\u0113${name}\u0100${value}\u0213\u0200\u0201\u030a`;
	  } else {
	    // TODO: button
	    // TODO: checkbox
	    // TODO: rang
	    // TODO: reset
	    // TODO: radio
	    // TODO: search
	    return ` [INPUT ${nid} ${type} ${name}: \u0116${value}\u0216]\u030a`;
	  }
	}
      })
    .replace(
      /<textarea[\s\S]*name=([^\s>]+)[\s\S]*?>([\s\S]*?)<\/textarea>/g,
      (a,id,val)=>{
	return `\n[WWW ${nid} ${id}\n${val}\n]\n`;
      })

    .replace(/<(\w+)[\s\S]*?>/g, '<$1>')

    .replace(
      /<code>(.*?)<\/code>/ig,
      '\u0102\u0110$1 \u0202\u0210')
    .replace(
      /<pre>([\s\S]*?)<\/pre>/gi,
      (a)=>'\u030a\u0110\u0102'+a.replace(/\n/g, '\u030a')
	.replace(/ /g, '\u0320')+
	'\u0202\u0210')

    .replace(/\n/g, ' ')
    .replace(/ +/g, ' ')

    .replace(/<li>\s*/g, '\n\u0101*\u0201')
    .replace(/<xxul>\s*/g, '\n&nbsp;&nbsp;' /* TODO: indent? */)
    .replace(/<p>\s*/g, '\n\u0320\u0320')
    .replace(/<\/p>\s*/g, '\n')
    .replace(/<div>/g, '\n')
    .replace(/<\/div>/g, '\n')
    .replace(/\n\n+/g, '\n\n')

    .replace(/<h1>([\s\S]*?)<\/h1>/g, '\n\u0110\u0107$1\u030a\u0210\u0207')
    .replace(/<h2>([\s\S]*?)<\/h2>/g, '\n\u0112\u0100$1\u030a\u0212\u0200')
    .replace(/<h3>([\s\S]*?)<\/h3>/g, '\n\u0113\u0100$1\u030a\u0213\u0200')
    .replace(/<h4>([\s\S]*?)<\/h4>/g, '\n\u0116\u0100$1\u030a\u0216\u0200')
    .replace(/<h5>([\s\S]*?)<\/h5>/g, '\n\u0116\u0101$1\u030a\u0216\u0201')
    .replace(/<h6>([\s\S]*?)<\/h6>/g, '\n\u0116\u0105$1\u030a\u0215\u0200')


    .replace(/<table>/ig, '{{{' /* TODO: maybe show table as outline? */)
    .replace(/<\/table>/ig, '}}}\u030a')
    .replace(/<tr>([\s\S]*?)<\/tr>/ig, '{$1}\u030a')
    .replace(/<\/td>\s*<td>/ig, ' | ')
    .replace(/(<td>|<\/td>)/ig, ' | ')
    .replace(/<td>/ig, ' | <th>')
    .replace(/<th>([\s\S+]*?)<\/th>/g, '\u0101$1\u0201')
    .replace(/(<th>\s*<\/th>)/g, ' | ')

    .replace(/<[\s\S]*?>/g, ' ')

    .replace(/ +/g, ' ')
    .replace(/ *\n */g, '\n')
    .replace(/ *([\0-\37]) */g, '$1')
    .replace(/^\n*/, '')

    .replace(/&nbsp;/ig, '\u0320')
    .replace(/&gt;/ig, '>')
    .replace(/&lt;/ig, '<')
    .replace(/&quot;/ig, '"')
    .replace(/&.squo;/ig, "'")
    .replace(/&.dquo;/ig, '"')
    .replace(/&lsaquo;/ig, '<')
    .replace(/&rsaquo;/ig, '>')
    .replace(/&ndash;/ig, '-')
    .replace(/&mdash;/ig, '--')
    .replace(/&circ;/ig, '^')
    .replace(/&tilde;/ig, '~')
    .replace(/&permil;/ig, '%o')
    .replace(/&amp;/ig, '&')
    .replace(/&eur;/ig, 'EUR')

    .trim();
  return h;

// TODO: double! didn't work...?
//    .replace(/<h1>([\s\S]*?)<\/h1>/g, '\n\u0110\u0107\x1aJ$1\n\n\u0210\u0207')

//    .replace(/(?<=[\u0000-\u0007])([\u0000-\u0007])/g, '$1')
//    .replace(/(?<=[\u0010-\u0017])([\u0000-\u0017])/g, '$1')

}

module.exports = { HTML2TELETEXT };
