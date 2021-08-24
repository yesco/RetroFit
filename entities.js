// HTML tables
// - https://www.freeformatter.com/html-entities.html
// w3 Complete list!
// - https://html.spec.whatwg.org/multipage/named-characters.html#named-character-references
// w3 JSON file!

let fs= require('fs');

let entities= JSON.parse(fs.readFileSync('entities.json'));

let s= Object.keys(entities).sort().map(n=>{
  let e=entities[n], c=e.characters;
  // some outputs need \ ...
  if (0) {
  if (c.length!=1) {
    console.log("%%ERROR: "+n+" "+c+" len="+c.length);
  }
  }
  if (c.match(' ')) {
    console.log("%%ERROR.space!: "+n+" "+c+" len="+c.length);
  }
  //SHIT case matters!
  console.log('"'+n+' '+c+'"');
//  console.log(n+'='+c);
});

