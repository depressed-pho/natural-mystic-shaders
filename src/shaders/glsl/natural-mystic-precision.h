// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_PRECISION_H_INCLUDED)
#define NATURAL_MYSTIC_PRECISION_H_INCLUDED

// https://github.com/depressed-pho/natural-mystic-shaders/issues/86
#if defined(GL_FRAGMENT_PRECISION_HIGH)
#  define prec_hm highp
#else
#  define prec_hm mediump
#endif

#endif
