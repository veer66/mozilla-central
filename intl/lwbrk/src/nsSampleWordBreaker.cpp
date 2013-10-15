/* -*- Mode: C++; tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*- */
/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */


#include "nsSampleWordBreaker.h"
#include "nsString.h"
#include <CoreFoundation/CoreFoundation.h>

nsSampleWordBreaker::nsSampleWordBreaker()
{
  bufferedText = nullptr;
  endOfWord = nullptr;
}
nsSampleWordBreaker::~nsSampleWordBreaker()
{
}

NS_IMPL_ISUPPORTS1(nsSampleWordBreaker, nsIWordBreaker)

bool nsSampleWordBreaker::BreakInBetween(
  const PRUnichar* aText1 , uint32_t aTextLen1,
  const PRUnichar* aText2 , uint32_t aTextLen2)
{
  NS_PRECONDITION( nullptr != aText1, "null ptr");
  NS_PRECONDITION( nullptr != aText2, "null ptr");

  if(!aText1 || !aText2 || (0 == aTextLen1) || (0 == aTextLen2))
    return false;

  bool is_break = (this->GetClass(aText1[aTextLen1-1]) != this->GetClass(aText2[0]));
  uint32_t bufferedTextLen;
  if(!is_break) {
    if(bufferedText != aText1) {
      bufferedText = aText1;
      bufferedTextLen = aTextLen1 + aTextLen2;
      if(nullptr != endOfWord)
        delete endOfWord;
      endOfWord = new uint8_t[bufferedTextLen];
      memset(endOfWord, false, bufferedTextLen * sizeof(uint8_t));
      CFStringRef string = CFStringCreateWithCharacters(kCFAllocatorDefault, bufferedText, bufferedTextLen);
      CFStringTokenizerRef tokref = CFStringTokenizerCreate(kCFAllocatorDefault, 
          string, CFRangeMake(0, bufferedTextLen), kCFStringTokenizerUnitWordBoundary, CFLocaleCopyCurrent());
      while(kCFStringTokenizerTokenNone != CFStringTokenizerAdvanceToNextToken(tokref)) {
        CFRange wordRange = CFStringTokenizerGetCurrentTokenRange(tokref);
        endOfWord[wordRange.location + wordRange.length] = true;
      }
      CFRelease(string);
      CFRelease(tokref);
    }
    is_break = endOfWord[aTextLen1];
  }
  return is_break;
}

#define IS_ASCII(c)            (0 == ( 0xFF80 & (c)))
#define ASCII_IS_ALPHA(c)         ((( 'a' <= (c)) && ((c) <= 'z')) || (( 'A' <= (c)) && ((c) <= 'Z')))
#define ASCII_IS_DIGIT(c)         (( '0' <= (c)) && ((c) <= '9'))
#define ASCII_IS_SPACE(c)         (( ' ' == (c)) || ( '\t' == (c)) || ( '\r' == (c)) || ( '\n' == (c)))
#define IS_ALPHABETICAL_SCRIPT(c) ((c) < 0x2E80) 

// we change the beginning of IS_HAN from 0x4e00 to 0x3400 to relfect Unicode 3.0 
#define IS_HAN(c)              (( 0x3400 <= (c)) && ((c) <= 0x9fff))||(( 0xf900 <= (c)) && ((c) <= 0xfaff))
#define IS_KATAKANA(c)         (( 0x30A0 <= (c)) && ((c) <= 0x30FF))
#define IS_HIRAGANA(c)         (( 0x3040 <= (c)) && ((c) <= 0x309F))
#define IS_HALFWIDTHKATAKANA(c)         (( 0xFF60 <= (c)) && ((c) <= 0xFF9F))
#define IS_THAI(c)         (0x0E00 == (0xFF80 & (c) )) // Look at the higest 9 bits

uint8_t nsSampleWordBreaker::GetClass(PRUnichar c)
{
  // begin of the hack

  if (IS_ALPHABETICAL_SCRIPT(c))  {
	  if(IS_ASCII(c))  {
		  if(ASCII_IS_SPACE(c)) {
			  return kWbClassSpace;
		  } else if(ASCII_IS_ALPHA(c) || ASCII_IS_DIGIT(c)) {
			  return kWbClassAlphaLetter;
		  } else {
			  return kWbClassPunct;
		  }
	  } else if(IS_THAI(c))	{
		  return kWbClassThaiLetter;
	  } else if (c == 0x00A0/*NBSP*/) {
      return kWbClassSpace;
    } else {
		  return kWbClassAlphaLetter;
	  }
  }  else {
	  if(IS_HAN(c)) {
		  return kWbClassHanLetter;
	  } else if(IS_KATAKANA(c))   {
		  return kWbClassKatakanaLetter;
	  } else if(IS_HIRAGANA(c))   {
		  return kWbClassHiraganaLetter;
	  } else if(IS_HALFWIDTHKATAKANA(c))  {
		  return kWbClassHWKatakanaLetter;
	  } else  {
		  return kWbClassAlphaLetter;
	  }
  }
  return 0;
}

nsWordRange nsSampleWordBreaker::FindWord(
  const PRUnichar* aText , uint32_t aTextLen,
  uint32_t aOffset)
{
  nsWordRange range;
  NS_PRECONDITION( nullptr != aText, "null ptr");
  NS_PRECONDITION( 0 != aTextLen, "len = 0");
  NS_PRECONDITION( aOffset <= aTextLen, "aOffset > aTextLen");

  range.mBegin = aTextLen + 1;
  range.mEnd = aTextLen + 1;

  if(!aText || aOffset > aTextLen)
    return range;

  uint8_t c = this->GetClass(aText[aOffset]);
  uint32_t i;
  // Scan forward
  range.mEnd--;
  for(i = aOffset +1;i <= aTextLen; i++)
  {
     if( c != this->GetClass(aText[i]))
     {
       range.mEnd = i;
       break;
     }
  }

  // Scan backward
  range.mBegin = 0;
  for(i = aOffset ;i > 0; i--)
  {
     if( c != this->GetClass(aText[i-1]))
     {
       range.mBegin = i;
       break;
     }
  }
  if(kWbClassThaiLetter == c)
  {
	// need to call Thai word breaker from here
	// we should pass the whole Thai segment to the thai word breaker to find a shorter answer
  }
  return range;
}

int32_t nsSampleWordBreaker::NextWord( 
  const PRUnichar* aText, uint32_t aLen, uint32_t aPos) 
{
  int8_t c1, c2;
  uint32_t cur = aPos;
  if (cur == aLen)
    return NS_WORDBREAKER_NEED_MORE_TEXT;
  c1 = this->GetClass(aText[cur]);
 
  for(cur++; cur <aLen; cur++)
  {
     c2 = this->GetClass(aText[cur]);
     if(c2 != c1) 
       break;
  }
  if(kWbClassThaiLetter == c1)
  {
	// need to call Thai word breaker from here
	// we should pass the whole Thai segment to the thai word breaker to find a shorter answer
  }
  if (cur == aLen)
    return NS_WORDBREAKER_NEED_MORE_TEXT;
  return cur;
}
