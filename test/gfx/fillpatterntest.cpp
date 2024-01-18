/**
  *  \file test/gfx/fillpatterntest.cpp
  *  \brief Test for gfx::FillPattern
  */

#include "gfx/fillpattern.hpp"
#include "afl/test/testrunner.hpp"


/** Test constructors, isBlank, isBlack. */

AFL_TEST("gfx.FillPattern:init:default", a)
{
    gfx::FillPattern aa;
    a.check("isBlank", aa.isBlank());
    a.check("isBlack", !aa.isBlack());
}

AFL_TEST("gfx.FillPattern:init:zero", a)
{
    gfx::FillPattern aa(uint8_t(0));
    a.check("isBlank", aa.isBlank());
    a.check("isBlack", !aa.isBlack());
}

AFL_TEST("gfx.FillPattern:init:one", a)
{
    gfx::FillPattern aa(1);
    a.check("isBlank", !aa.isBlank());
    a.check("isBlack", !aa.isBlack());
}

AFL_TEST("gfx.FillPattern:init:ff", a)
{
    gfx::FillPattern aa(0xFF);
    a.check("isBlank", !aa.isBlank());
    a.check("isBlack", aa.isBlack());
}

AFL_TEST("gfx.FillPattern:init:multiple-ffs", a)
{
    static const uint8_t black[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
    gfx::FillPattern aa(black);
    a.check("isBlack", aa.isBlack());
    a.check("isBlank", !aa.isBlank());
}

AFL_TEST("gfx.FillPattern:init:bytes", a)
{
    static const uint8_t notQuiteBlack[] = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0x00};
    gfx::FillPattern aa(notQuiteBlack);
    a.check("isBlack", !aa.isBlack());
    a.check("isBlank", !aa.isBlank());
}


/** Test operators. */
AFL_TEST("gfx.FillPattern:operators", a)
{
    static const uint8_t pattern[] = {0x01,0,0,0,0,0,0x30,0};
    gfx::FillPattern aa(pattern);
    const gfx::FillPattern& bb(aa);
    a.check("01. isBlank", !aa.isBlank());
    a.check("02. isBlack", !aa.isBlack());
    a.checkEqual("03. init", aa[0], 1);
    a.checkEqual("04. init", aa[1], 0);
    a.checkEqual("05. init", aa[2], 0);
    a.checkEqual("06. init", aa[3], 0);
    a.checkEqual("07. init", aa[4], 0);
    a.checkEqual("08. init", aa[5], 0);
    a.checkEqual("09. init", aa[6], 0x30);
    a.checkEqual("10. init", aa[7], 0);

    a.checkEqual("11. init const", bb[0], 1);
    a.checkEqual("12. init const", bb[1], 0);
    a.checkEqual("13. init const", bb[2], 0);
    a.checkEqual("14. init const", bb[3], 0);
    a.checkEqual("15. init const", bb[4], 0);
    a.checkEqual("16. init const", bb[5], 0);
    a.checkEqual("17. init const", bb[6], 0x30);
    a.checkEqual("18. init const", bb[7], 0);

    aa.shiftLeft(2);
    a.checkEqual("21. shiftLeft", aa[0], 4);
    a.checkEqual("22. shiftLeft", aa[1], 0);
    a.checkEqual("23. shiftLeft", aa[2], 0);
    a.checkEqual("24. shiftLeft", aa[3], 0);
    a.checkEqual("25. shiftLeft", aa[4], 0);
    a.checkEqual("26. shiftLeft", aa[5], 0);
    a.checkEqual("27. shiftLeft", aa[6], 0xC0);
    a.checkEqual("28. shiftLeft", aa[7], 0);

    aa.shiftUp(3);
    a.checkEqual("31. shiftUp", aa[0], 0);
    a.checkEqual("32. shiftUp", aa[1], 0);
    a.checkEqual("33. shiftUp", aa[2], 0);
    a.checkEqual("34. shiftUp", aa[3], 0xC0);
    a.checkEqual("35. shiftUp", aa[4], 0);
    a.checkEqual("36. shiftUp", aa[5], 4);
    a.checkEqual("37. shiftUp", aa[6], 0);
    a.checkEqual("38. shiftUp", aa[7], 0);

    aa.shiftRight(5);
    a.checkEqual("41. shiftRight", aa[0], 0);
    a.checkEqual("42. shiftRight", aa[1], 0);
    a.checkEqual("43. shiftRight", aa[2], 0);
    a.checkEqual("44. shiftRight", aa[3], 6);
    a.checkEqual("45. shiftRight", aa[4], 0);
    a.checkEqual("46. shiftRight", aa[5], 0x20);
    a.checkEqual("47. shiftRight", aa[6], 0);
    a.checkEqual("48. shiftRight", aa[7], 0);

    aa.shiftDown(1);
    a.checkEqual("51. shiftDown", aa[0], 0);
    a.checkEqual("52. shiftDown", aa[1], 0);
    a.checkEqual("53. shiftDown", aa[2], 0);
    a.checkEqual("54. shiftDown", aa[3], 0);
    a.checkEqual("55. shiftDown", aa[4], 6);
    a.checkEqual("56. shiftDown", aa[5], 0);
    a.checkEqual("57. shiftDown", aa[6], 0x20);
    a.checkEqual("58. shiftDown", aa[7], 0);

    aa.flipVertical();
    a.checkEqual("61. flipVertical", aa[0], 0);
    a.checkEqual("62. flipVertical", aa[1], 0x20);
    a.checkEqual("63. flipVertical", aa[2], 0);
    a.checkEqual("64. flipVertical", aa[3], 6);
    a.checkEqual("65. flipVertical", aa[4], 0);
    a.checkEqual("66. flipVertical", aa[5], 0);
    a.checkEqual("67. flipVertical", aa[6], 0);
    a.checkEqual("68. flipVertical", aa[7], 0);

    aa.flipHorizontal();
    a.checkEqual("71. flipHorizontal", aa[0], 0);
    a.checkEqual("72. flipHorizontal", aa[1], 4);
    a.checkEqual("73. flipHorizontal", aa[2], 0);
    a.checkEqual("74. flipHorizontal", aa[3], 0x60);
    a.checkEqual("75. flipHorizontal", aa[4], 0);
    a.checkEqual("76. flipHorizontal", aa[5], 0);
    a.checkEqual("77. flipHorizontal", aa[6], 0);
    a.checkEqual("78. flipHorizontal", aa[7], 0);

    aa.invert();
    a.checkEqual("81. invert", aa[0], 0xFF);
    a.checkEqual("82. invert", aa[1], 0xFB);
    a.checkEqual("83. invert", aa[2], 0xFF);
    a.checkEqual("84. invert", aa[3], 0x9F);
    a.checkEqual("85. invert", aa[4], 0xFF);
    a.checkEqual("86. invert", aa[5], 0xFF);
    a.checkEqual("87. invert", aa[6], 0xFF);
    a.checkEqual("88. invert", aa[7], 0xFF);

    aa ^= 0xF0;
    a.checkEqual("91. op^=", aa[0], 0x0F);
    a.checkEqual("92. op^=", aa[1], 0x0B);
    a.checkEqual("93. op^=", aa[2], 0x0F);
    a.checkEqual("94. op^=", aa[3], 0x6F);
    a.checkEqual("95. op^=", aa[4], 0x0F);
    a.checkEqual("96. op^=", aa[5], 0x0F);
    a.checkEqual("97. op^=", aa[6], 0x0F);
    a.checkEqual("98. op^=", aa[7], 0x0F);

    static const uint8_t toggle[] = {0x08,0x1B,0x18,0x0B,0x08,0x1B,0x18,0x0B};
    aa ^= gfx::FillPattern(toggle);
    a.checkEqual("101. op^=", aa[0], 0x07);
    a.checkEqual("102. op^=", aa[1], 0x10);
    a.checkEqual("103. op^=", aa[2], 0x17);
    a.checkEqual("104. op^=", aa[3], 0x64);
    a.checkEqual("105. op^=", aa[4], 0x07);
    a.checkEqual("106. op^=", aa[5], 0x14);
    a.checkEqual("107. op^=", aa[6], 0x17);
    a.checkEqual("108. op^=", aa[7], 0x04);

    aa |= 0x40;
    a.checkEqual("111. op|=", aa[0], 0x47);
    a.checkEqual("112. op|=", aa[1], 0x50);
    a.checkEqual("113. op|=", aa[2], 0x57);
    a.checkEqual("114. op|=", aa[3], 0x64);
    a.checkEqual("115. op|=", aa[4], 0x47);
    a.checkEqual("116. op|=", aa[5], 0x54);
    a.checkEqual("117. op|=", aa[6], 0x57);
    a.checkEqual("118. op|=", aa[7], 0x44);

    static const uint8_t bits[] = {0x08,0x02,0x02,0x08,0x08,0x02,0x02,0x08};
    aa |= gfx::FillPattern(bits);
    a.checkEqual("121. op|=", aa[0], 0x4F);
    a.checkEqual("122. op|=", aa[1], 0x52);
    a.checkEqual("123. op|=", aa[2], 0x57);
    a.checkEqual("124. op|=", aa[3], 0x6C);
    a.checkEqual("125. op|=", aa[4], 0x4F);
    a.checkEqual("126. op|=", aa[5], 0x56);
    a.checkEqual("127. op|=", aa[6], 0x57);
    a.checkEqual("128. op|=", aa[7], 0x4C);

    aa &= 0x11;
    a.checkEqual("131. op&=", aa[0], 0x01);
    a.checkEqual("132. op&=", aa[1], 0x10);
    a.checkEqual("133. op&=", aa[2], 0x11);
    a.checkEqual("134. op&=", aa[3], 0x00);
    a.checkEqual("135. op&=", aa[4], 0x01);
    a.checkEqual("136. op&=", aa[5], 0x10);
    a.checkEqual("137. op&=", aa[6], 0x11);
    a.checkEqual("138. op&=", aa[7], 0x00);

    static const uint8_t mask[] = {0xFF,0xF0,0x0F,0xFF,0xF0,0x0F,0xFF,0xF0};
    aa &= gfx::FillPattern(mask);
    a.checkEqual("141. op&=", aa[0], 0x01);
    a.checkEqual("142. op&=", aa[1], 0x10);
    a.checkEqual("143. op&=", aa[2], 0x01);
    a.checkEqual("144. op&=", aa[3], 0x00);
    a.checkEqual("145. op&=", aa[4], 0x00);
    a.checkEqual("146. op&=", aa[5], 0x00);
    a.checkEqual("147. op&=", aa[6], 0x11);
    a.checkEqual("148. op&=", aa[7], 0x00);
}

/** Test predefined patterns. */
AFL_TEST("gfx.FillPattern:predefined", a)
{
    a.check("01", gfx::FillPattern::SOLID.isBlack());
    a.check("02", !gfx::FillPattern::SOLID.isBlank());

    a.check("11", !gfx::FillPattern::GRAY50.isBlack());
    a.check("12", !gfx::FillPattern::GRAY50.isBlank());

    a.check("21", !gfx::FillPattern::GRAY25.isBlack());
    a.check("22", !gfx::FillPattern::GRAY25.isBlank());

    a.check("31", !gfx::FillPattern::GRAY50_ALT.isBlack());
    a.check("32", !gfx::FillPattern::GRAY50_ALT.isBlank());

    a.check("41", !gfx::FillPattern::LTSLASH.isBlack());
    a.check("42", !gfx::FillPattern::LTSLASH.isBlank());
}
