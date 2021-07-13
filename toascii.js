// Fragment: map unicode characters into 7-bit ASCII glyphs

  let scandimap = {'Æ': 'AE', 'Å': "A'", 'å':"a'", 'Ä':'A"', 'ä':'a"', 'Ð':'D-', 'ð':'TH', 'Ø':':0/', 'ø':'o/', 'Ö':'O"', 'ö':'o"', 'Þ':'TH|', 'þ':'TH|'};
  let scandis = RegExp('([ÆÅåÄäÐðØøÖöÞþ])', 'g');

  // debug latin-1... lol
  // .replace(/([^\x20-\x7f])/ig,
  //   c=>c+'['+c.charCodeAt(0)+']')
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
