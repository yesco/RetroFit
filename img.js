function img2teletext(tag, src, alt, lid, n) {
  // TODO: generalize
  // if (alt === 'Google') {
  // TODO:
  //
  // } else { 
    // TODO: change default to:
    // - background tall letters
    // - with spacing (depend width?)
    // - generate TELETEXT of alt-text
    // - generate lores image
    return `\u0116${n}:${alt?alt:''} \u0216`;
  }
}
