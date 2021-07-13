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
