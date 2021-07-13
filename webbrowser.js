  function INVERT(...args) {
    // DOC: New: Invert part of the screen
    // INVERT row, col, len
    // INVERT "STR"
    // INVERT ' - turns off all

    let lochar = c=>String.fromCharCode(c.charCodeAt(0)&0x7f);
      
    if (args.length === 0) {
      fill(0, 0, SCREEN_ROWS, SCREEN_COLS,
	   (r,c)=>{
	     let ch = screen(r, c);
	     let c7 = ch & 0x7f;
	     if (c7 !== ch) screen(r, c, c7);
	   });
    } else if (args.length === 3) {
      let [r,c,len] = args;
      // set hibit
      for(let i=0; i<len; i++)
	screen(r, c+i, screen(r, c+i)|0x80);
    } else if (args.length === 1) {
      let s = args[0];
      let len = s.length;
      // protect regexp chars
      s = s.replace(/([^\w])/g, '\\$1');

      for(let r=1; r<SCREEN_ROWS; r++) {
	getScreenLine(r).replace(
	  RegExp(s, 'g'), (a,i)=>
	    INVERT(r, i, len));
      }
    }
  }

  function doAction(action) {
    INVERT();

    // go one down line
    print();

    try {
      if (action.www)
	return urlView(action.www);
      else if (action.linktext) {
	// TODO:
      } else if (action.button)
	return run(action.button);
    } catch (e) {
      alert('doAction.error: ' + e + '\n' +
	    e.stack);
    }
  }

  // only return match if cursor at it
  function atmatch(s, re, dbg) {
    let res;
    s.replace(RegExp(re, 'g'), (a,m,i)=>{
      if (i <= col && col <= i+m.length)
	res = m;
      if (dbg) alert('atmatch: i='+i+' m='+m);
    });
    return res;
  }

  function findAction() {
    try {

      // TODO: use alt inverse so we don't deastroy inverse on the screen by user
      INVERT();
      let r = xfindAction();
      if (r) {
	if (r.linktext) {
	  let id = `${wget.pageid} ${r.linktext}`;
	  let url = wget.url[id];
	  //alert('Lookup '+r.linktext+'\n='+url);
	  if (url) {
	    r.www = url;
	  } else {
	    // partial match...
	    let re = RegExp(
	      r.linktext.replace(/([^\w])/g, '\\$1'));
	    let mk = Object.keys(wget.url)
		.filter(k=>k.match(re));
	    if (!mk.length) {
	      r.msg = 'link not matched';
	    } else if (mk.length == 1)
	      r.www = wget.url[mk[0]];
	    else { // choices
	      //r.msg = 'too many matches';
	      //r.www = m[0]; // what the heck
	    }
	  }
	  
	  if (r.www) r.type = 'WWW';
	}

	// fix url (if all uppercase)
	if (r.www) {
	  if (r.www === r.www.toUpperCase())
	    r.www = r.www.toLowerCase();
	}

	let txt = r.button || r.www || r.msg;
	if (!txt) {
	  txt = Object.keys(r).filter(
	    k=>k.match(/(www|button|linktext)/))
	      .map(
		k=>`${k.toUpperCase()} ${r[k]}`).join('');
	}

	txt = r.type + ' ' + txt;
	clearline(0);
	iplot(0, 0, txt.substr(0, SCREEN_COLS));

	INVERT(r.hilite);
      }
      return r;

    } catch(e) {alert(e);}
  }
  function xfindAction() {
    // TODO:
    // - CTRL-U should go to top bar and
    // - show current URL, return there
    // - should go to it

    let ln = getScreenLine(row, 'nohi');

    // - [!WWW 1 3] - button
    let button = atmatch(
      ln,
      /(\[!{0,1}[^\]]+\])/);
    if (button) return {
      button: rget(button, /\[!{0,1}(.*)\]/)[0],
      hilite: button,
      type: 'button',
    };

    // - blue link name under cursor
    // (ok this is sketchy: 'blue')
    // (what if change markup?)
    // match till next attr
    // '\4link\4link\0'
    // TODO: attr inside linktext
    let linktext = atmatch(
      ln, /(?<=[\4\6])\s*([^\0-\37]{2,})\s*[\0-\037]/);
    if (linktext) return {
	type: 'linktext',
	linktext,
	hilite: linktext,
    }

    // try till EOL
    linktext = atmatch(
      ln, /[\4\6]\s*([^\0-\37]{2,})\s*$/g);
    if (linktext) return {
      type: 'linktext',
      linktext,
      hilite: linktext,
      partial: true,
    }

    // - under/before around current cursor
    let atCursor = atmatch(
      ln, /([^\0-\37\s]{3+)/);
    if (atCursor) {
      // - url under cursor (more strict)
      let [url] = rget(
	atCursor,
	/\b([\w\.\/\?:&=\+%\-\"\'!\$~@#\^\*_]{3,}[^\.])\b/i);
      if (url && url.match(/\S\.\S/))
	return {
	  type: 'WWW',
	  www: url,
	  hilite: url,
	};
    }

    // nothing
    return;
  }

  function editEnter() {
    // DOC: Run line buffer command
    // New: If emtpy find [button] at cursor or URL
    cursorHide(); // turned on by run/stop
    if (line) {
      print();
      // TODO: right place? lol
      // (we hide it in case of error)
      dom('exp').value = line;
      // run turns on cursor if appropriate
      // and not if background running
      run(line);
      line = '';
      return;
    } else {
      let action = findAction();
      if (action)
	return doAction(action);
    }
    print();
    cursorShow();
  }

  // basically make other arguments splittable
  function quoteStrings(line) {
    return line.replace(
      /\".?\"/g,
      s=>s.replace(/!/g, '!!')
	.replace(/ /g, '!s!')
	.replace(/'/g, '!q!')
	.replace(/:/g, '!c!')
      	.replace(/\(/g, '!po!')
        .replace(/=/g, '!e!')
        .replace(/,/g, '!com!')
    );
  }
  function unquoteStrings(line) {
    return line.replace(
      s=>s.replace(/!c!/g, ':')
	.replace(/!s!/g, ' ')
      	.replace(/!po!/g, '(')
        .replace(/!pc!/g, ')')
        .replace(/!e!/g, '=')
        .replace(/!q!)/g, "'")
        .replace(/!com!)/g, ',')
	.replace(/!!/g, '!'));
  }

  function prompt(noprompt) {
    if (!noprompt && !running()) {
      // why was added? because break?
      //if (col > 2)
      print();
      print('Ready');
      cursorShow();
    }
  }

  // interprets and runs one line
  // returns true if ok
  // END return false
  function run(line) {
    if (!line) return;

    // special commands
    if (line.match(/^\S+\.\S/))
      return urlView(line);
    // TODO: [button]?
    // TODO: NAME [.COM] (any file...)
    // TODO: !EXTNAME [.COM] (new funcs)
    if (line.match(/^.{2,}\?$/)) {
      return urlView(
	'http://html.duckduckgo.com/html/?q=' +
	  line.replace(/\?/, ''));
    }
    if (line.match(/^wiki (\w+)$/i)) {
      return urlView(
	('HTTP://EN.M.WIKIPEDIA.ORG/WIKI/' + 
	  line.replace(/^wiki\s*/i, ''))
	 .toLowerCase());
    }


    run.count++;

    // NN PRINT ... new line?
    // New: retains initial spacing! lola
    let linedef = line.match(/^\s*(\d+)(.*)$/);
    if (linedef) {
      saveline(+linedef[1], linedef[2]);
      cursorShow();
      return;
    }

    // commands to execute
    let p = parser(line);
    try {
      let r = statements(p);
      if (r) prompt();
      return r;
    } catch(e) {
      if (e === 'SILENT' || e && e.oric)
	cursorShow();
      else 
	run_error(e);
    }
  }
  run.count = 0;
  
  function editBackspace() {
    cursorHide();
    // Oric: move to newline if...
    if (!line) {
      if (col > column) print();
    } else {
      // delete char
      putc(8);
      line = line.substring(0, line.length-1);
    }
    cursorShow();
  }
  
  function editCancel() {
    line = ''; 
    cursorHide();
    princ('\\');
    print();
    cursorShow();
  }

  function editCopy() {
    // DOC: CTRL-A copies char from screen into edit buffer
    // New: If it's 0--31 - an attribute it's prefixed by ESC (27)
    // ORIC programs can handle and display ESC and attributes so encoded
    let c = screen(row, col);
    // escape read attributes
    if ((c & 0x7f) < 32) {
      editChar(27);
    }
    editChar(c);
  }

  let arrows = {l:37, u:38, r:39, d:40};
  function cursorMove(direction, steps=1) {
    direction = direction.toLowerCase();
    let c = arrows[direction[0]];
    while(steps--)
      editArrow(c);
  }

  function uglies(f) {
    let c = f( screen(row, col) );
    // wrap around
    if (c >= 96) c = c - 64;
    if (c < 32) c = c + 64;
    screen(row, col, c);
    let ch = String.fromCharCode(c);
    // build strange bitmap
    let bits = c.toString(2).padStart(7, '0');
    // skip 2nd highest bit (32)
    bits = bits[0]+bits.substr(-5);
    bits = bits.split('').reverse().join('')
      .match(/(..)/g)
      .join(' ');
    clearline(0);
    iplot(2,0,`CHAR '${ch}' ${HEX$(c)} (${c}) ALT: \u0009${ch}\u0008 ${bits}`);
  }

  // This is probably a crazy idea!
  // adding full TELETEXT EDITING!
  function editExt(k, kc, a, s, c, e) {
    // DOC: New! Extended full screen editing!
    // CTRL-A copy ATTR/chr at cursor
    // ALT-G  enter GEDIT mode!
    // CTRL-H DEL char (scroll left)
    // CTRL-I INS spc (scroll right)
    // CTRL-J toggle protected col 2
    // CTRL-M CR, move to col 2
    // CTRL-P toggle "printer" log
    // CTRL-R reload sim (reset)
    // CTRL-Y yanks (pastes) the edit
    // CTRL-BS delete line, scroll up
    // CTRL-ENTER insert line, " down
    // CTRL-arrow copy ATTR to dir
    // CTRL-? help for color ATTRs
    // CTRL-1 insert INK 1 (red)
    // ALT-3  insert PAPER 3 (yellow)
    // CTRL-ALT toggle show ATTRs
    // CTRL-ALT-U "uglies" charcode+1
    // - TELETEXT/ALT block chars
    // CTRL-SHIFT-1 toggle bit 1
    // CTRL-SHIFT-6 toggle bit 6
    
    //e.preventDefault();
    try{
      cursorHide();
      //if (!k.match(/[0-9a-z]/)) return;
      //alert(`editExt: '${k}' ${kc} ${a?'a':''} ${s?'s':''} ${c?'c':''}`);

      // TODO:
      switch(k) {
      //case 'v': alert("editExt: can't catch 'v"); break;
      
	// ins char shift right after cursor
      case 'i': 
	if (c) {
	  for(let i=SCREEN_COLS-1; i>col; i--)
	    screen(row, i, screen(row, i-1));
	  screen(row, col, 32);
	}
	break;

	// DEL shift left after cursor
      case 'h':
	if (c) {
	  for(let i=col; i<SCREEN_COLS-1; i++)
	    screen(row, i, screen(row, i+1));
	  screen(row, SCREEN_COLS-1, 32);
	}
	break;
      case '/': // unsifted ? lol
      case '?': 
	clearline(0);
	iplot(0,0,'COLOR:CTRL- ALT- 0 \21'+'1\0\22'+'2 \23'+'3\7\24'+'4 \25'+'5\0\26'+'6 \27'+'7\0');
	break;
      
	// Yank! (paste line buffer)
	// (ALT-Y stays, and move down)
      case 'y': {
	let sr=row, sc=col;
	princ(line);
	if (a) {
	  row=sr+1; col=sc;
	  if (row >= SCREEN_ROWS) row=0;
	}
	break; }

	// uglies - block graphics
      case 'u': if (c && a) uglies(x=>x+1); break;

	// fg: CTRL-# bg: ALT-#
	// (and CTRL-ALT-# uglies)
      case (k.match(/^\d$/)?k:false): {
	// uglies - block graphics
	// C-A-#   1 2
	//         3 4
	//         5 6
	// 0 = 00 00 00
	// 7 = 11 11 11
	// 8 = 10 01 10
	if (c && a) {
	  kc = kc - 48; // 0--9
	  uglies(x=>{
	    switch(kc) {
	    case 0: return 32+0;
	    case 7: return 32+1+2+4+8+16+32;
	    case 8: return x ^ (95-32);;
	    case 9: return x ^ (1+4+16);
	    case 6: return x + 32; // !
	    default: return x ^ (1<<(kc-1));
	    }
	  });
	  break;
	}
	// colors CTRL ->  0-- 7 (fg)
	// colors ALT  -> 16--23 (bg)
	editChar(ESC);
	k = kc-48 + (a?16:0);
	editChar(k + 64); // ESC A = \1
	break; }
      }
      //alert('EDITEXT: '+k + ' kc='+kc);
      cursorShow();
    } catch(err){throw err; alert(
      'ERROR: ' + err +
	`\neditExt: '${k}' ${kc} ${a?'a':''} ${s?'s':''} ${c?'c':''}`);}
    
  }
  
  // A graphical edit mode!
  // ======================
  // https://www.oric.org/software/lorigraph-1241.html
  // TODO: Lorigraph was apparently one of
  // the better drawing programs for ORIC
  // Features:
  // - pixel (' ', 1, 0)
  // - line (l)
  // - line-line-line (l arrow l arrow l))
  // - star (back to origin) (l o arrow l o)
  // - rectangle (m arrow r)
  // - ROMB! (TODO:)
  // - attributes (TODO:)
  // - block (?) (TODO:)
  // - swap (TODO:)
  // - copy (TODO:)
  // - mirror (TODO:)
  // - chars (symbols, fonts)
  // - text (oric) (TODO:)
  // - erase (TODO:)
  // - invert (TODO:) kind of a "fill"
  // - rotation (TODO:)
  // - flip (rot, mirror, unrot) (TODO:)
  // - fill (I'm assuming 'paint') (TODO:)
  // - print (TODO:)
  // - save (TODO:)
  // - load (TODO:)
  // - pattern (TODO:)
  // - paper
  // - ink
  // - write/erase (xor?)

  // this contains state as well as impl
  var gCMD = {
    ox: undefined,
    oy: undefined,
    mode: 2, // xor
    xmode: 0, // crosshair
    step: 1,

    stack: [], // lol
    index: 0,

    ops: [],

    saved: undefined,

    // colors will be multiples of 8+color...
    ink: 7, 
    paper: 0,

    // const - don't save

    xmodes: '.x+/-# ',

    // functions as map
    // (all functions are added by name too!)
    //    l: functinon draw()... key='l'
    //    L: fucntion... key = 'L'
    //  A_l: fun... key = 'ALT-l'
    //
    // it can be called by can be called by
    //   gCMD.apply(gCMD, 'l');
    //   gCMD.apply(gCMD, 'draw');
    //
    // screen update func: call save('func')
    // this enables 'replay'!
    
    save: function save(name) {
      this.ops.push([name, this.getState()]);
    },
    getState: function getState() {
      return {
	curx, cury,
	ox: this.ox, oy: this.oy,
	mode: this.mode,
	xmode: this.xmode,
	ink: this.ink,
	paper: this.paper,
      };
    },
    applyState: function applyState(s) {
      curx = s.curx; cury = s.cury;
      this.ox = s.ox; s.oy = s.oy;
      this.mode = s.mode;
      this.ink = s.ink;
      this.paper = s.paper;
    },
    A_r: function replay() {
      // TOOD: save text?
      // TODO: save and restore stack
      CLS(); LORES(1);
      console.log('replay.1');
      this.ops.forEach(op=>{
	console.log('replay.2');
	let o = op[0];
	let s = op[1];
	console.log('replay.3 op=' + op + ' ' + JSON.stringify(s));
	this.applyState(s);
	console.log('replay.4');
	// replay command!
	(this[o])(); // no args
	console.log('replay.5');
      });
      console.log('replay.6 done');
    },
    h: function home() {
      this.ox = curx;
      this.oy = cury;
      if (hires()) {
	;
      } else {
	curx = INT((SCREEN_COLS-1) * 2 / 2);
	cury = INT((SCREEN_ROWS-1) * 3 / 2);
	this.mark();
      }
    },
    ' ': function toggle() {
      this.save('toggle');
      CURMOV(0, 0, 2);
      this.mark();
    },
    0: function unset() {
      this.save('unset');
      CURMOV(0, 0, 0);
      this.mark();
    },
    1: function set() {
      this.save('set');
      CURMOV(0, 0, 1);
      this.mark();
    },
    M: function modechange() {
      // 0 - unset
      // 1 - set (if it's no attribute)
      // 2 - xor
      // 3 - just move
      // (4 - don't clobber text (LORES 1)
      this.mode = (this.mode + 1) % 3;
      return 'Mode = ' + this.mode;
    },
    m: function mark() {
      // don't mark twice (keep other)
      // erh? this is a noop..
      // keep for thirdx...
      if (this.ox == curx && this.oy == cury)
	return;
      this.ox = curx;
      this.oy = cury;
      this.push();
    },
    o: function other() {
      let x=curx; curx=this.ox; this.ox=x;
      let y=cury; cury=this.oy; this.oy=y;
      CURSET(curx, cury, 3);
    },
    push: function push() {
      let last = this.stack[this.stack.length-1];
      // don't push if already at top
      if (last && last.x == curx && last.y == cury)
	return;

      this.stack.push({x: curx,y: cury});
      this.index = this.stack.length-1;
      return this.stack.length;
    },
    pop: function pop() {
      let r = this.stack.pop();
      if (!r) return 'No more positions!';
      
      CURSET(r.x, r.y, 3);
      return this.stack.length;
    },
    p: function prev() {
      if (this.index > 0) this.index--;
      let pos = this.stack[this.index];
      CURSET(pos.x, pos.y, 3);
      return this.index;
    },
    n: function next() {
      if (this.index+1 < this.stack.length)
	this.index++;
      let pos = this.stack[this.index];
      CURSET(pos.x, pos.y, 3);
      return this.index;
    },
    B: function block() {
      // DOC: fill all with background color
      try{
	let old = this.paper % 8 + 16
      this.paper++;
      fill(1, 0, SCREEN_ROWS-1, SCREEN_COLS,
	   (r,c)=>{
	     let ch = screen(r, c);
	     // this erases attr of same color :-(
	     if (ch === 32 || ch  == old) 
	       return this.paper % 8 + 16;
	     else
	       return ch;
	     });
} catch(e) {alert(e);}
    },
    L: function lores() {
      if (hires())
	return "Can't initiate LORES in HIRES";
      this.ink = ink;
      this.paper = paper;
      LORES();
    },
    l: function line() {
      this.save('line');
      this.other();
      this.push();
      DRAW(this.ox-curx, this.oy-cury, this.mode);
      this.push();
      this.mark();
    },
    c: function circle() {
      this.save('circle');
      sq = (v)=>v*v;;
      let r = SQR(sq(this.ox-curx) + sq(this.oy-cury));
      this.other();
      CIRCLE(r, this.mode);
      this.other();
      // save current pos
      this.push();
      return INT(r);
    },
    r: function rect() {
      this.save('rect');
      let x = curx, y = cury;
      this.other();
      DRAW(this.ox-x, 0, this.mode);
      DRAW(0, this.oy-y, this.mode);
      DRAW(x-this.ox, 0, this.mode);
      DRAW(0, y-this.oy, this.mode);
      this.other();
    },
    I: function Ink() {
      INK(++this.ink % 8);
    },
    P: function Paper() {
      PAPER(++this.paper % 8);
    },
    q: function quit() {
      gedit = false;

      cursorShow();
      setTimeout(showStatus, 500);
      return 'BYE!';
    },
    // also works as init function
    init: function init() {
      cursorHide();

      if (typeof this.ox === 'undefined') {
	// init if first time only
	this.h();
	this.ox = curx;
	this.oy = cury;
      }

      gedit = true;
    },
    // TODO: https://www.vt100.net/docs/vt3xx-gp/chapter1.html#S1.6
    // - diamond
    // - crosshair
    // - rubber band line (show what will be drawn)
    // - rubber band rectangle (selection?)
    x: function crossmode() {
      this.xmode = (this.xmode+1) % this.xmodes.length;
      return this.xmodes[this.xmode].repeat(10);
    },
    cross: function cross() {
      // save
      let x=curx;
      let y=cury;

      let m = this.xmodes[this.xmode];
      switch(m) {
      case '.': CURSET(x, y, 2); break;
      case '+':   // long cross
      case 'x': { // straight cross
	let z = m == 'x' ? 5 : 300;
	CURSET(x, y, 3); DRAW(+z, 0, 2);
	CURSET(x, y, 3); DRAW(-z, 0, 2);
	CURSET(x, y, 3); DRAW( 0,+z, 2);
	CURSET(x, y, 3); DRAW( 0,-z, 2);
	break; }
      case '/': { // pen
	let z = 5;
	CURSET(x, y, 3); DRAW(+z+1,-z-1, 2);
	CURSET(x+1, y-2, 3); DRAW(+z,-z, 2);
	CURSET(x+2, y-1, 3); DRAW(+z,-z, 2);
	// we need to draw x,y even times!
	CURSET(x, y, 2);
	break; }
      case '-': { // rubber line
	CURSET(this.ox, this.oy, 3);
	DRAW(x-this.ox, y-this.oy, 2);
	break; }
      case '#': { // rubber box
	let dx = x-this.ox;
	let dy = y-this.oy;
	CURSET(this.ox, this.oy, 3);
	DRAW(dx, 0, 2);
	DRAW(0, dy, 2);
	DRAW(-dx, 0, 2);
	DRAW(0, -dy, 2);
	break; }
      default: // none;
      }

      // restore
      CURSET(x, y, 3);
    },
    // show other "cross"
    ocross: function ocross(x, y) {
      let sx=curx;
      let sy=cury;
      let m = this.xmodes[this.xmode];
      if (m !== 'x' && m !== '+') return;

      let z = m == 'x' ? 3 : 300;
      CURSET(x, y, 3); DRAW(+z,+z, 2);
      CURSET(x, y, 3); DRAW(-z,-z, 2);
      CURSET(x, y, 3); DRAW(+z,-z, 2);
      CURSET(x, y, 3); DRAW(-z,+z, 2);

      CURSET(sx, sy, 3);
    },
    arrowup: function(){cury-=this.step},
    arrowdown: function(){cury+=this.step},
    arrowleft: function(){curx-=this.step},
    arrowright: function(){curx+=this.step},

    CA_arrowup: function(){cury-=this.step},
    CA_arrowdown: function(){cury+=this.step},
    CA_arrowleft: function(){curx-=this.step},
    CA_arrowright: function(){curx+=this.step},

    // unhifted ?
    '/': function(){this.help()},
    '?': function help() {
      // toggle - restore if saved
      if (this.saved) {
 	screenDraw(this.saved);
	this.saved = '';
	return;
      } else {
	this.saved = screenSaved();
      }

      // show help
      let h =
'\33\3\33\24'+`G(raphical) EDIT`+'\33\20\33\7'+`
  by (c) Jonas S Karlsson

A raw screen editor (LORES 1)
- text and attr line editing
- graphical TELETEXT drawing

SHIFT/ALT-arrow move 10/25

----------------------------------------
move    h-home     o-other   arrow-keys
        p-prev     n-next    CTRL/SHIFT

pixel   SPC-toggle 1-set     0-unset
draw    l-line     c-circle  r-rect
fill               C         R

----------------------------------------
---------x---------x---------x---------x
color    I-Ink     P-Paper 
m-mode   L-lores 

q-quit  ?-back to edit  q-quit

---------x---------x---------x---------x
CTRL-    ALT-         G(raphical)EDIT   ?Turtle
   C-? help               ? help
=================  =====================
                   SPC toggle pixel     
-A copy   copywrd? a?copy                
-B  -     Blink?   b?brush  
-C break  clipart? c circle  C?fill    
-D double Double?                       down  
-E TEXTRA export?  e?erase   E?export       
-F keyclck file?   f?fill    F?Flip     forw
---------x---------x---------x---------x
-G bell   GEDIT                         
-H del    horiz    h home                
-I insert ?Inv?icol          I ink
-J column job?                       
-K reKall  "" kill?                         
-L CLS     list?   l line    L LORES    left
-M |<--    menu?   m mark    M?mode
-N clrline new?    n next       
-O out-off open?   o other   OtherThird
-P printer         p prev    P paper    
---------x---------x---------x---------x  
-Q cursor quote?   q quit          
-R reload Rotate?  r rect    R?fill     rig/rot
-S scstop Scroll?  s?swap               shw set
-T caps   term?    t?text    T?textfont times
-U url    URL?     u                    up
-V paste  view?    v
-W write           w
-X cancel Xut?     x xhair                
-Y yank   Yank     y
-Z?undo   Zoom?    z zigzagline cursor
---------x---------x---------x---------x
-? color help      ? help
-0 blacK bkground  0 unset
-1 Red       "     1 set
-2 Green     "     
-3 Blue      "
-4 Yellow    "
-5 Magnenta  "
-6 Cyan      "
-7 White     "     
-8 Textmode        8 twist?
-9 altGrph         9 rotate/curl/spiral?
---------x---------x---------x---------x
C-arr copy step25 step 1    step 10
C-RET insert line
C-BS delete line
---------x---------x---------x---------x
# repeat
[ ( start macro
] ) end macro
1 
2
3
4
5
6
7
8
9
0
space toggle color on in blockmode
A attribute?
B black Black
C cycle Cycle
D double?
E 
F fill?
G graphics?
H horizontal? Horizontal?
I invert? ink?
J jump?
K klean? kill?
L line? list?
M mode? move?
N new?w
O open?
P paper?
Q quit Quit?
R red Red
S screen? scroll??
T text? Text?
U 
V vertical? Vertical?
W white White
X crosshair? x-out? xecute?
Y yank (paste)
Z zoom? Zoom?
`;
      CLS();
      print(h);
      return;
      // print K-name ...
      Object.keys(this).filter(
	k=>k.length==1).sort().map(
	  k=>k+'-'+this[k].name)
	.forEach(s=>{
	  if (s.length + POS() >= SCREEN_COLS)
	    TAB();
	  princ(s); TAB();
	});

    },

  };
  // make all functions keys too!
  Object.keys(gCMD).forEach(
    k=>{ let f = gCMD[k];
	 if (f && f.name)
	   gCMD[f.name]=f;
       });

  // TODO: hiresman
  // -- https://forum.defence-force.org/viewtopic.php?f=14&t=905

  function GEDIT(k, kc, a, s, c, e) {
    // DOC: G(raphical) EDIT!
    // For HIRES or LORES mode
    // Simple key commands to draw
    // ALT-G to activate
    // ?-for help, q-quit
    let g = gCMD;

    function msg(txt) {
      clearline(0);
      iplot(0, hires()?27:0, txt);
    }
    
    // undraw if needed
    if (gedit) {

      // undraw x
      g.cross();

      // undraw o
      if (typeof GEDIT.ox !== 'undefined')
	g.ocross(GEDIT.ox, GEDIT.oy);
      GEDIT.ox = undefined;
    }

    // increase step length!
    if (s)
      g.step = 10;
    else if (a)
      g.step = 25;
    else
      g.step = 1;

    // undo
    //if (c && g.last) g.last.call(g);
    //if (c) g.circle();

    if (s && k.length===1)
      k=k.toUpperCase();
    // if alt key do prefix by CAS_key
    if (a)
      k=(c?'C':'')+(a?'A':'')+(s?'S':'')+'_'+k;

    let f = g[k];
    console.log('GEDIT: ' + k + ' ' + (f?f.name:'nope'));
    if (!f) {
      msg('GEDIT: ?-help q-quit');
    } else {
      msg('GEDIT.' + f.name);
      let r;
      try  {
	r = f.call(g, k, kc, a, s, c, e);
      } catch(e) {
	alert('GEDIT.'+f.name+': ERROR\n'+e);
      }
      if (r)
	msg('GEDIT.' + f.name + ': ' + r);
      
      // update pos in right corner
      let pos = '\3'+`${ABS(curx-g.ox)} ${ABS(cury-g.oy)}`+'\2'+`${curx} ${cury}`;
      iplot(40-pos.length, hires()?27:0, pos);
    }
   
    // save for undo
    //if (c) g.circle();
    
    //if (c && g.last) {
    //g.last.call(g);
  //} else {
    //g.last = f;
  //}

    if (hires()) {
      ; //
    } else {
      // update cursor to same cell!
      col = 1 + INT(curx / 2) % SCREEN_COLS;
      row = 1 + INT(cury / 3) % SCREEN_ROWS;
      if (col < 0) col += SCREEN_COLS;
      if (row < 0) row += SCREEN_ROWS;
    }

    // if we're exiting don't draw!
    if (!gedit) return;

    // draw o
    GEDIT.ox = g.ox;
    GEDIT.oy = g.oy;
    if (typeof GEDIT.ox !== 'undefined')
      g.ocross(GEDIT.ox, GEDIT.oy);

    // draw x
    g.cross();
  }

  function editArrow(ch) {
    switch(ch) {
    case 37: col--; break; // left
    case 38: row--; break; // up
    case 39: col++; break; // right
    case 40: row++; break; // down
    }
    // move cursor, wrap around (?)
    if (col < column) col = SCREEN_COLS-1
    if (col > SCREEN_COLS-1) col = column;
    if (row < 0) row = SCREEN_ROWS-1;
    if (row > SCREEN_ROWS-1) row = 0;

    findAction();
  }

  function clearline(optRow, optCol) {
    //return;
    // clears current line (or given)
    // (optional from col)
    optCol = (optCol===undefined) ? column : optCol;
    optRow = (optRow===undefined) ? row : optRow;
    iplot(
      optCol, optRow,
      SPC(SCREEN_COLS-optCol));
    // reset colors
    if (optCol === column) {
      iplot(0, optRow, paper+16)
      iplot(1, optRow, ink);
    }
  }

  function showStatus() {
    function update(col, status, msg) {
      if (col < 0) col += SCREEN_COLS;
      let endcol = col + msg.length;
      if (0 && endcol >= SCREEN_COLS)
	msg = msg.substr(
	  0, msg.length - (endcol-SCREEN_COLS));
      iplot(col, 0,
	   status ? msg : SPC(msg.length));
    }

    // difficult to update if OFF!
    // (lol we update even if off :-)
    // (ctrl-o on ORIC stopps updates)
    let saved = scoff; scoff = 0;
    
    clearline(0);
    
    // gEdit mode
    update(0, gedit, 'GEDIT');

    // CTRL-Q: toggle cursor
    update(-4, caps, 'CAPS');

    // CTRL-P: toggle printer
    update(-4 -1 -7, printer, 'PRINTER');
    
    // CTRL-O: toggle screen (centered)
    update(Math.floor(SCREEN_COLS-10)/2,
	     saved, 'SCREEN OFF');

    // CTRL-S: toggle screen (centered)
    if (scstop) {
      // only overwrite status ofscoff if scstop
      update(Math.floor(SCREEN_COLS-12)/2,
	     scstop, 'SCREEN PAUSE');
    }

    // restore, no write to screen
    scoff = saved;
  }
  
  function writeScreen() {
    // now to give it a name?
    // string put in 0,0? lol?
    let sc = screenSaved();
    let name = sc.name || 'noname';
    let id = 'SCREEN:' + name + ' ' + sc.date;
    sc.id = id;

    let dispid = sc.date.replace(/\..*$/, '')
	.replace(/[:-]/g, '')
	.replace('T', ' ');
    dispid = name + ' ' + dispid;
    clearline(0); iplot(0, 0, dispid);
    sc.dispid = dispid;

    let msg = lsput(id, JSON.stringify(sc)) ?
	'\2--SAVED' : '\1--FAILED';
    iplot(dispid.length, 0, msg);
    // TODO: make current state as not dirty
  }

  function readScreen() {
    // TODO: if dirty,save first!
    let q = readScreen.queue;
    if (!q || !q.length) {
      q = readScreen.queue = lssearch('SCREEN:');
    }
    let id = q.pop();
    let sc = lsget(id);
    if (sc) {
      sc = JSON.parse(sc);
      screenDraw(sc);
      let dispid = sc.dispid;
      if (dispid) {
	clearline(0);
	iplot(0, 0, dispid);

	let index = ' ' + q.length;
	iplot(SCREEN_COLS-index.length, 0, index);
      }
    } else {
      clearline(0); iplot(0,0,'\1FAIL: '+id);
    }
  }

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

  let scandimap = {'Æ': 'AE', 'Å': "A'", 'å':"a'", 'Ä':'A"', 'ä':'a"', 'Ð':'D-', 'ð':'TH', 'Ø':':0/', 'ø':'o/', 'Ö':'O"', 'ö':'o"', 'Þ':'TH|', 'þ':'TH|'};
  let scandis = RegExp('([ÆÅåÄäÐðØøÖöÞþ])', 'g');

  // debug latin-1... lol
  // .replace(/([^\x20-\x7f])/ig,
  //   c=>c+'['+c.charCodeAt(0)+']')
 

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
    .replace(/\ufffd/g, '(?)' /*unknown*/)

    .replace(/\u00B0/g, 'o' /* degree */)
    .replace(/\u200b/g, '' /* zwidth space */)

    .replace(/\u2100/g, 'a/o')
    .replace(/\u2101/g, 'a/s')
    .replace(/\u2103/g, 'oC')
    .replace(/\u2105/g, 'c/o')
    .replace(/\u2106/g, 'c/u')
    .replace(/\u2109/g, 'oF')
    .replace(/\u214d/g, 'A/S')

    .replace(/\u2116/g, 'No ')
    .replace(/\u2121/g, 'TEL ')
    .replace(/\u2122/g, 'TM')
    .replace(/\u2126/g, 'Ohm')
    .replace(/\u212a/g, 'K')
    .replace(/\u212b/g, 'Angstro"m')
    .replace(/\u213b/g, 'FAX ')

    .replace(/\u218e/g, '&lt;-')
    .replace(/\u218f/g, 'v')

    .replace(/\u2190/g, '&lt;-')
    .replace(/\u2191/g, '^')
    .replace(/\u2192/g, '-&gt;')
    .replace(/\u2193/g, 'v')
    .replace(/\u2194/g, '&lt;-&gt;')
    .replace(/\u2195/g, '(^|v)')
    .replace(/\u2196/g, '(^\)')
    .replace(/\u2197/g, '(/^)')
    .replace(/\u2198/g, '(\v)')
    .replace(/\u2199/g, '(v/)')
    .replace(/\u219e/g, '&lt;&lt;-')
    .replace(/\u219f/g, '^|^')
    .replace(/\u21a0/g, '-&gt;&gt;')
    .replace(/\u21a1/g, 'v|v')
    .replace(/\u21a2/g, '&lt;-&lt;')
    .replace(/\u21a3/g, '&gt;-&gt;')
    .replace(/\u21a4/g, '&lt;-|')
    .replace(/\u21a5/g, '^|.')
    .replace(/\u21a6/g, '|-&gt;')
    .replace(/\u21a7/g, "'|v")
    .replace(/\u21a9/g, "&lt;-'")
    .replace(/\u21aa/g, "'-&gt;")
    .replace(/\u21ab/g, "'-&gt;")
    .replace(/\u21b0/g, "&lt;-,")
    .replace(/\u21b1/g, ",-&gt;")
    .replace(/\u21b2/g, "&lt;-'")
    .replace(/\u21b3/g, "'-&gt;")

    .replace(/\u21b6/g, '(anticlockwise)')
    .replace(/\u21b7/g, '(clockwise)')
    .replace(/\u21ba/g, '(anticlockwise)')
    .replace(/\u21bb/g, '(clockwise)')

    .replace(/\u21d0/g, '&lt;==')
    .replace(/\u21d1/g, '|^|')
    .replace(/\u21d2/g, '==&gt;')
    .replace(/\u21d3/g, '|v|')
    .replace(/\u21d4/g, '&lt;==&gt;')

    .replace(/\u21e4/g, '|&lt-')
    .replace(/\u21e5/g, '-&gt;|')

  
    .replace(/\u2200/g, '(forall)')
    .replace(/\u2202/g, '(partial difference)')
    .replace(/\u2203/g, '(there exists)')
    .replace(/\u2204/g, '(there does not exists)')
    .replace(/\u2205/g, '(emtpy set)')
    .replace(/\u2206/g, '(inc)')
    .replace(/\u2207/g, '(dec)')
    .replace(/\u2208/g, '(element of)')
    .replace(/\u2209/g, '(not element of)')
    .replace(/\u220b/g, '(contains)')
    .replace(/\u220c/g, '(does not contain)')

    .replace(/\u220e/g, 'QED' /* end of proof */)
    .replace(/\u220e/g, '(".)' /* end of proof */)
    .replace(/\u220f/g, '(N*)' /* N-ary product */)
    .replace(/\u220g/g, '(U*)' /* U-ary coproduct */)
    .replace(/\u220g/g, '(E+)' /* N-ary summation */)

    .replace(/\u2212/g, '-')
    .replace(/\u2213/g, '+/-')
    .replace(/\u2215/g, '/')
    .replace(/\u2217/g, '*')
    .replace(/\u2218/g, 'o' /* ring operator */)
    .replace(/\u221a/g, 'SQRT')
    .replace(/\u221b/g, 'CUBEROOT')
    .replace(/\u221c/g, '4ROOT')
    .replace(/\u221e/g, '(oo)')
    .replace(/\u221f/g, '(L)')
    .replace(/\u2220/g, '(V)')
    .replace(/\u2221/g, '(V-)')
    .replace(/\u2222/g, '(&t;|)')
    .replace(/\u2223/g, '(DIVIDES)')
    .replace(/\u2224/g, '(DOES NOT DIVIDE)')
    .replace(/\u2225/g, '||')
    .replace(/\u2226/g, '(-||)')
    .replace(/\u2227/g, '(^)')
    .replace(/\u2228/g, '(v)')
    .replace(/\u2229/g, '(&')
    .replace(/\u222a/g, '(U)')
    .replace(/\u222b/g, '(INTEGRAL)')
    .replace(/\u222c/g, '(2INTEGRAL)')
    .replace(/\u222d/g, '(3INTEGRAL)')
    .replace(/\u2234/g, "(.'.)")
    .replace(/\u2235/g, "('.')")
    .replace(/\u2236/g, ':')
    .replace(/\u2237/g, '::')

    .replace(/\u2260/g, '!=')
    .replace(/\u2261/g, '===')
    .replace(/\u2262/g, '!==')
    .replace(/\u2263/g, '(==)')
    .replace(/\u2264/g, '&lt;=')
    .replace(/\u2265/g, '&gt;=')
    .replace(/\u226a/g, '&lt;&lt;')
    .replace(/\u22d8/g, '&lt;&lt;&lt;')
    .replace(/\u226b/g, '&gt;&gt;')
    .replace(/\u22d9/g, '&gt;&gt;&gt;')
    .replace(/\u2276/g, '&lt;&gt;')
    .replace(/\u2277/g, '&gt;&lt;')

    .replace(/\u2282/g, '(SUBSET OF)')
    .replace(/\u2283/g, '(SUPERSET OF)')

    .replace(/\u22bb/g, '(XOR)')
    .replace(/\u22bc/g, '(NAND)')
    .replace(/\u22bd/g, '(NOR)')

    .replace(/\u22c8/g, '|&gt;&lt;')


    .replace(/\u00d7/g, 'x')
    .replace(/\u00b7/g, '-' /* middle dot */)
    .replace(/\u22c5/g, '(.)' /* dot operator */)

    .replace(/\u2002/g, ' ' /* en-space, lol */)

    .replace(/\u00BB/g, '&gt;&gt;')
    .replace(/\u203a/g, '&gt;')
    .replace(/[\u2018-\u201b]/ig, "'")
    .replace(/[\u201c-\u201f]/ig, '"')

    .replace(/[\u0300\u0301\u0307\u0309\u030a\u030c-\u030e\u0310-\u0315\u031a\u0319]/g, "'")
    .replace(/[\u030b\u030f]/g, '"')
    .replace(/[\u0324\u032a-\u032f\u033a]/g, '__')
    .replace(/[\u0316-\u0319\u031c-\u0323\u0334-\u0345\u0339]/g, ',')
    .replace(/[\u0335-\u0336]/g, '-')
    .replace(/[\u0337-\u0338]/g, '/')

    .replace(/[\u0300-\u036f]/g, '^^' /* all accent crap! */)

    .replace(/\u0320/ig, '-')
    .replace(/\u2010/ig, '-')
    .replace(/\u2011/ig, '-')
    .replace(/\u2012/ig, '-')
    .replace(/\u2013/ig, '--')
    .replace(/\u2014/ig, '---')
    .replace(/\u2015/ig, '--')
    .replace(/\u2016/ig, '||')
    .replace(/\u2017/ig, '_')
    .replace(/\u2018/ig, '_')
    .replace(/\u2020/ig, '+')
    .replace(/\u2021/ig, '++')
    .replace(/\u2022/ig, '*')
    .replace(/\u2023/ig, '|&gt;')
    .replace(/\u2023/ig, '|&gt;')
    .replace(/[\u2024\u2027]]/ig, '.')
    .replace(/\u2025/ig, '..')
    .replace(/\u2026/ig, '...')
    .replace(/\u2028/ig, '\n')
    .replace(/\u2029/ig, '<p>')
    .replace(/\u2030/ig, '%o')
    .replace(/\u2031/ig, '%oo')
    .replace(/[\u2032\u2035]/ig, "'")
    .replace(/[\u2033\u2036]/ig, "''")
    .replace(/[\u2034\u2037]/ig, "'''")

    .replace(/\u2045/ig, '[-')
    .replace(/\u2046/ig, '-]')
    .replace(/\u2047/ig, '??')
    .replace(/\u2048/ig, '?!')
    .replace(/\u2049/ig, '!?')
    .replace(/\u2049/ig, '!?')

    .replace(/\u204c/ig, '*|')
    .replace(/\u204d/ig, '|*')
    .replace(/[\u204e\u2045]/ig, '*')
    .replace(/\u2053/ig, '~')

    .replace(/\u2038/ig, '^')
    .replace(/\u2039/ig, '&lt;')    .replace(/\u203a/ig, '&gt;')
    .replace(/\u203b/ig, '***')
    .replace(/\u203c/ig, '!!')
    .replace(/\u203d/ig, '!?')
    .replace(/\u2044/ig, '/')

    .replace(/\u2056/ig, '.:')
    .replace(/\u2057/ig, "''''")
    .replace(/\u2058/ig, '.:.')
    .replace(/\u2059/ig, ':.:')
    .replace(/\u205a/ig, ':')
    .replace(/\u205b/ig, '.:.')
    .replace(/\u205c/ig, '.+.')
    .replace(/\u205d/ig, ':::')

    .replace(/\u00e0/ig, "'a")
    .replace(/\u00e8/ig, "'e")
    .replace(/\u00e9/ig, "e'")

    .replace(/\u2014/g, '-'/*nobrdash*/)
    .replace(/\ufeff/g, '')
    .replace(scandis,
	     (a,c)=>(scandimap[c] || '\\?'))
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
	wget.url[`${nid} ${txt}`] = href;
	let lid = `${nid} ${n}`;
	wget.url[lid] = href;
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

      // TODO: similar to <a ...
      wget.url[`${nid} ${txt}`] = src;
      let lid = `${nid} ${n}`;
      wget.url[lid] = src;
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
	  wget.form[wget.pageid][name] = value;
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

// TODO: Wired.com break something
// TODO: ibm.com too much empty lines
function urlView(url) {
  CLS();
  clearline(0);
  iplot(0, 0, 'Loading '+url+' ...');
  let start = Date.now();
  wget(
    url,
    function(h, err, nid, url) {
      showPage(h, err, nid, url,
	       Date.now()-start);
    });
}

function showPage(h, err, nid, url, loadms){
  clearline(0);
  iplot(0, 0, 'load='+loadms);

  let parsems = Date.now();
  let txt = HTML2TELETEXT(nid, h);
  parsems = Date.now() - parsems;

  let isink = c7=>(c7 <= 7);
  let ispaper = c7=>(16 <= c7 && c7 <= 24);

  // fast go pages back
  let pageindex = [];
  pageindex.push([0,0]);
  let lastcleanindex = 0;

  // state
  let i, screenstartindex, step,
      cink, cpaper, cstack,
      lastwasink, lastwaspaper, lastwasspace,
      rowcounter, lastrow,
      startTime,

      forwardms, renderms,

      lastDummy;

  i = 0;

  step = SCREEN_COLS; //SCREEN_COLS; // !
  step = SCREEN_ROWS*SCREEN_COLS/1;

  function reset(atindex=-1, lci=-1) {
    // This controls how many chars to print
    // per tick.
    //  1 = 300 baud (feels like it, readble)
    //  4 = 1200 baud
    //  SCREEN_COLS = about 1 page / second

    // set-up screen
    CLS();
    iplot(0, 0,
	  (url || '').substring(0, SCREEN_COLS));
    startTime = Date.now();

    screenstartindex = atindex;

    forwardms = -Date.now();

    if (i <= atindex) {
      // we're good, were already on the way
      
      // enable the runner
      inputMode = undefined;
    } else {

      // we're starting from beginning!
      i = 0;
      if (atindex >= lastcleanindex) {
	i = lastcleanindex;
      }
      if (lci !== -1 &&
	  atindex >= lci) {
	lastcleanindex = i = lci;
      }

      screenstartindex = atindex>=0 ? atindex : 0;
      // charcodes of current inc/paper
      cink = ink;
      cpaper = paper + 16;

      // color stack
      cstack = [];
    
      lastwasink = false;
      lastwaspaper = false;
      lastwasspace = (ch == 32);

      rowcounter = 0;
      lastrow = 0;
    }
  }
  function redraw() {
    reset(screenstartindex);
  }
  
  reset();

  // TODO: make it the default
  if (cink !== 7 || cpaper !== 0+16) {
    column = 2;
  }
  
  function putc(ch) {
    let lastrow = row;
    window.putc(ch);
    // if not time to show, just care about the attributes
    if (i <= screenstartindex) {
      // TODO: we just keep (over) writing it at the same row!
      if (row > 1) {
	row = 1;
	screen(1, 0, paper+16);
	screen(1, 1, ink);
      }
    } else {
      if (forwardms < 0) {
	forwardms = Date.now() + forwardms;
	renderms = -Date.now();
      } 
    }
  };

  function myputc(ch) {
    // enable for debug
    if (0) {
      lastwasspace = false;
      lastwasink = false;
      lastwaspaper = false;
    }

    if (row != lastrow) {
      rowcounter++;
      lastrow = row;
    }

    if (cink == 7 && cpaper == 0+16 && col <= column && (lastwasink || lastwaspaper || lastwasspace)) {
      col = 0;
      column = 0;
    }

    // TODO: make it the default
    if (cink !== 7 || cpaper !== 0+16) {
      column = 2;
      if (col < column) col = column;
    }
    //console.log('putc '+JSON.stringify({ c: String.fromCharCode(ch), ch, col, cpaper, cink, lastwasink, lastwaspaper, lastwasspace }));


    // horunge (bastad childs)
    // TOOD: fix ugly word breaks
    // TODO: cheat at .,:,? at EOL?

    let c7 = ch & 0x7f;

    // optimize corner cases
    // if at beginning of line and space
    if (col == column && (ch == 32 || ch == 10)) {
      // ignore such space!
      return;
    } else if (col == column
	       && (isink(c7) || ispaper(c7))){
      // save space by using protected
      //let cont = (ch & 0x0F00 != 0x0300);
      if (isink(c7)) {
	screen(row, 1, cink = c7);
      } else if (ispaper(c7)) {
	screen(row, 0, cpaper = c7);
      }
      // it can only be overwritten if column
      lastwasink = false;
      lastwaspaper = false;
      lastwasspace = false

      return; // done here
    } else if (ch == 32 && lastwasspace) {
      // ignore soft spaces
      return;
    }

    if (ch == 32 && (lastwasspace || lastwasink || lastwaspaper))
      return;

    // escape control codes
    if (ch != 10 && ch != 27 && ch < 32) {
      // TODO: some bug if enable lastwassapce
      // HTML.COM/TABLES losses some bgcol restore
      // this is because, they may come in different order, the end attribute actually just triggers restore of cink/cpaper...
      
      if ((lastwasspace && 0) ||
	  (lastwasink && isink(c7)) ||
	  (lastwaspaper && ispaper(c7))) {
	// how about more levels... lol
	if (col > 2) col--; // BackSpace
      }
      if (lastwasspace && col > 0 &&
	  screen(row, col-1) === ' ') {
	col--; // BackSpace
      }
      // update
      if (isink(c7)) {
	cink = c7;
	lastwasink = true;
	lastwaspaper = false;
	lastwasspace = false;
      }
      if (ispaper(c7)) {
	cpaper = c7;
	lastwasink = false;
	lastwaspaper = true;
	lastwasspace = false;
      }

      // TODO: 107 is start white ink
      // how about inverse?
      putc(ESC);
      putc(c7+64);
      // TODO: if write at last pos...?
      lastwasspace = true;
      // no return as this might have wrapped line and we need fall through for cols
    } else {
      // finally print it!
      lastwasink = false;
      lastwaspaper = false;
      lastwasspace = (ch == 32);

      // hard newline (pre, br)
      if (ch == 0x030a) ch = 10;
      // hard space (pre)
      if (ch == 0x0320) ch = 32;

      // paper 0 ink 7
      // (<h1>) can just move cursor to col=0!
      if (col == column && (lastwasink || lastwasspace || lastwaspaper) && cink == 7 && cpaper == 0+16) {
	col = 0;
      }
      //lastwasspace = false;

      putc(ch);
      //putc('.'.charCodeAt(0));
      //if (ch > 128 || ch < 32) princ('['+ch+']');
    }

    // at newline need continue color
    if (col == column) {
      // but only if default is different
      if (ink != cink)
	myputc(cink);
      if (paper != cpaper)
	myputc(cpaper);
    }
  }

  function navigate() {
    renderms = Date.now() + renderms;
    // stats line
    clearline(0);
    iplot(0,0,'l'+loadms+' f'+forwardms+' r'+renderms+' c'+lastcleanindex+' i'+i+'/'+txt.length+' h'+h.length);
    renderms = -Date.now();
    let percent = INT(100 * screenstartindex /txt.length);
    let percent2 = INT(100 * i /txt.length);
    let page = pageindex.length;
    let pages = INT(100 * pageindex.length / percent2 +0.5);
    let posmsg = '\x00\x13'+page+'/'+pages+' '+percent2+'%';
    iplot(SCREEN_COLS-posmsg.length, 0, posmsg);

    // help line
    iplot(0, SCREEN_ROWS-1, // bottom row
	  '\x13\x04WWW\x03\x03\x14 SPC B < + - ESC C-U 319  \x13\x04??? ');

    inputMode = function navigation(k) {
      if (k < 0) return;

      let c = String.fromCharCode(k).toLowerCase();
      //iplot(0,0, 'k='+k+' c='+c+'    ');

      // less style commands?
      switch(c) {
      case 'f':
      case ' ': { // forward page
	inputMode = undefined;
	// continue where we left off
	let g = lastcleanindex; // i-50;
	pageindex.push([g,g]);
	reset(g, g);
	break; }
      case 'b': { // back page
	iplot(0,SCREEN_ROWS-1,'back............');
	inputMode = undefined;
	// continue where we left off
	if (!pageindex.length)
	  pageindex.push([0,0]);
	let g;
	[g, lastcleanindex] = pageindex.pop();
	if (!pageindex.length)
	  pageindex.push([0,0]);
	reset(g, lastcleanindex);
	break; }
      case 'q':
      case ESC: { // quit
	inputMode = undefined;
	CLS(); // TODO: restore screen?
	lastKey = -1;
	stop();
	return true;
	break; }
      case '<': { // beginning
	inputMode = undefined;
	//step = txt.length;
	reset(0);
	break; }
      case '/': // search in page
	inputMode = undefined;
	// fall through to next
      case 'n': { // next match
	inputMode = undefined;
	reset(0);
	break; }
      case 'p': { // prev match
	inputMode = undefined;
	reset(0);
	break; }
      case '+': { // zoom: less rows/cols
	inputMode = undefined;
	// how about state? (parse from beginning?)
	//step = txt.length;
	TEXT(INT(SCREEN_ROWS/1.4+1),
	     INT(SCREEN_COLS/1.1));
	redraw();
	break; }
      case '-': { // zoom out: more rows/cols
	inputMode = undefined;
	// how about state? (parse from beginning?)
	//step = txt.length;
	TEXT(INT(SCREEN_ROWS*1.4),
	     INT(SCREEN_COLS*1.1));
	redraw();
	break; }
      case '3': // 300 baud!
	step = SCREEN_COLS/20; redraw(); break;
      case '1': // 1200 baud!
	step = SCREEN_COLS; redraw(); break;
      case '9': // full speed!
	step = SCREEN_COLS*SCREEN_ROWS*10; redraw(); break;
      default: { // pass it on (arrows etc)
	return false;
	break; }
      }

      // eat up keystroke
      lastKey = -1;
      end = i + step;
      return true;
    };

    //row = toprow;
    //col = column;

    return;
  }

  RUN.interval = setInterval(
    function printSlow() {
      // TOOD: generalize these runners
      // for LIST, RUN, CLOAD, WWW
      if (scstop) return;
      if (Date.now() < waittill) return;
      if (inputMode) return;

      let end = i + step;
      for(; i<end; i++) {

	// it's a clean state! - remember!
	if (cstack.length == 0 &&
	    col == 2 &&
	    cink == ink && cpaper == paper+16) { 
	  lastcleanindex = i;
	}


	if (row >= SCREEN_ROWS-2)
	  return navigate();

	if (i > txt.length) {
	  if (col !== column) print();
	  print('-- ' + url);

	  // TODO: not accurate as involve waiting time for next page etc
	  //print('(Rendering '+(Date.now()-startTime)+' ms)');
	  return navigate();
	}

	let c = txt[i];
	let ch = txt.charCodeAt(i);
	let c7 = ch & 0x7f;

	// output whole word unbroken
	// or split word according to hyphen.
	// (this already avoids already hypnen)
	// word-limit length == 25...
	let word;
	if (1)
	  //if (c === '"' || c === "'" ||
	  //(c >= 'a' && c <= 'z'))
	{
	  word = txt.substr(i, 25)
	    .match(/^['"]{0,1}[\w]+[,\.\?:;!'"]{0,1}/);
	}
	if (word) {
	  word = word[0];
	  let l = word.length;

	  //myputc('['.charCodeAt(0));

	  if (col + l < SCREEN_COLS) {
	    // fits!
	    for(let j=0; j<l; j++)
	      myputc(word.charCodeAt(j));
	    //myputc('~'.charCodeAt(0));
	  } else if (1) {
	    // doesn't fit
	    let parts = hyphen(word).split(' ');
	    parts.forEach((p,i)=>{
	      let islast = parts.length === i+1;
	      let pl = p.length;
	      // newline if not fit, and not too big
	      if (islast &&
		  col + pl <= SCREEN_COLS){
		// last does fit!
		; // fine!
	      } else {
		// isn't last, or didn't fit

		if (i)
		  myputc('-'.charCodeAt(0));

		// '-' could have moved us!
		if (col !== column) {
		  myputc(0x030a);
		  //myputc('&'.charCodeAt(0));
		}

	      }
	      
	      for(let j=0; j<pl; j++)
		myputc(p.charCodeAt(j));
	      //myputc('&'.charCodeAt(0));
	    });
	  }

	  //myputc(']'.charCodeAt(0));

	  i +=  l - 1; // cont will inc by one!
	  //end += 2;
	  continue;
	}
	
	if (ch >= 0x0300) {
	  // continue color (delimiter)
	  myputc(ch);
	} else if (ch >= 0x0200) {
	  // end color
	  let t = cstack.pop();
    
	  if (!t) {
	    // ingore unmatched
	    //myputc(32);
	    myputc('?'.charCodeAt(0));
	    continue;
	  }

	  if (0) {
	    // allow reverse in any order
	    // (trust end attribute)
	    myputc(c7);
	  } else {
	    // reverse in matching order
	    // resore color
	    if (cpaper != t.cpaper) {
	      myputc(t.cpaper);
	      //myputc('P'.charCodeAt(0));
	    }
	    if (cink != t.cink) {
	      myputc(t.cink);
	      //myputc('C'.charCodeAt(0));
	    }
	  }

	  if (0) {
	    // TODO: graphics ???
	    myputc(32);
	  }
	  //princ(`[i:${t.cink}p:${t.cpaper}]`);
	} else if (ch >= 0x0100) {
	  // start color
	  cstack.push({c7, cink, cpaper});
	  if (c7 == cink || c7 == cpaper) {
	    // same
	    myputc(32);
	  } else {
	    // hack: color correction:
	    // (<h1><a>... => blue on black :-(
	    // change to cyan!
	    if (cpaper == 0+16 && c7 == 4)
	      c7 = 6;
	    myputc(c7);
	  }
	  //myputc('v'.charCodeAt(0));
	} else {
	  myputc(ch);
	}
	
      } // for
    }, 0); // set interval
} // 

/* WORD-SPLITTING
 https://english.stackexchange.com/questions/385/what-are-the-rules-for-splitting-words-at-the-end-of-a-line

Here they are, in roughly decreasing order of priority:

- Break words at morpheme boundaries (inter-face, pearl-y, but ear-ly).
- Break words between doubled consonants — 'sc' counts here but not 'ck'. (bat-tle, as-cent, jack-et).
- Never separate an English digraph (e.g., th, ch, sh, ph, gh, ng, qu) when pronounced as a single unit (au-thor but out-house).
- Never break a word before a string of consonants that cannot begin a word in English (anx-ious and not an-xious).
- Never break a word after a short vowel in an accented syllable (rap-id but stu-pid).
- Finally, if the above rules leave more than one acceptable break between syllables, use the Maximal Onset Principle:

// RULES
// - Never slit Names with Capitals
//   (first word?)
// - Don't start line w 2 letter suffix
// - Don't hyphen word with hyphen!
//

// A simple algorithm in code on:
// - https://stackoverflow.com/questions/405161/detecting-syllables-in-a-word 

*/
  
  // hyphenate -> hy phen ate
  // ignores leading/ending interpunktuations
  //
  // errors:
  //   authentication -> au then tic a tion
  //
  // probably crap, but simple!
  // but better than nothing!(?)
  function hyphen(w) {
    // don't hypenate Proper Names (or aNyFunny)
    if (w.match(/[A-Z]/)) return w;
    // no hypernate hypernated words!
    if (w.match(/\-/)) return w;

    // remove pre/post non characters (add later)
    let pre = '', post = '';
    w = w
      .replace(/^(\W+)/, a=>(pre=a,''))
      .replace(/(\W+)$/, a=>(post=a,''))

    // exceptions
    w = w
      .replace(/^(pro|pre|de|un|dis|post|be|para|hy|all)/, '$1 ');
    
    // wovelS consonant - 
    w = w.replace(/([aeiouy]+[bcdfgjklmnpqrstvwxz][he]{0,1})/g, '$1 ');

    // undo some...
    w = w
      .replace(/ +/g, ' ').trim()
      .replace(/\b ([aeiouy])([^aeiouy])([aeiouy]) /g, ' $1 $2')
      .replace(/c k/g, 'ck ')
      .replace(/n g/g, 'ng')
      .replace(/t h/g, 'th')
      .replace(/\b ([^aeiouy])\b/g, '$1')
      .replace(/\b([^aeiouy]) \b/g, '$1')
      .replace(/ es$/, 'es' /* silent */)
      .replace(/ (.)ing$/, '$1 ing')
      .replace(/(.) ion$/, ' $1ion')
      .replace(/(.) ied$/, ' $1ied')
      .replace(/(.) al$/, ' $1al')
      .replace(/(.) st$/, ' $1st')
      .replace(/(.) ry$/, ' $1ry')
      .replace(/\Bth /, ' th')
      .replace(/(\w[^aeiouy])ed/, '$1 ed')
      .replace(/(\w[aeiouy])ed/, ' $1 ed')
      .replace(/r y$/, ' ry')
      .replace(/^(\S{1,3}) /, '$1')
      .replace(/ (\S{1,3})$/, '$1')
    ;
    return pre + w + post;
  }

