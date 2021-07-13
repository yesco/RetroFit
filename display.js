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
