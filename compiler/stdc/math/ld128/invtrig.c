/*-
 * Copyright (c) 2008 David Schultz <das@FreeBSD.ORG>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

//__FBSDID("$FreeBSD: src/lib/msun/ld128/invtrig.c,v 1.1 2008/07/31 22:41:26 das Exp $");

#include "ld128/invtrig.h"

/*
 * asinl() and acosl()
 */
const long double
pS0 =  1.66666666666666666666666666666700314e-01L,
pS1 = -7.32816946414566252574527475428622708e-01L,
pS2 =  1.34215708714992334609030036562143589e+00L,
pS3 = -1.32483151677116409805070261790752040e+00L,
pS4 =  7.61206183613632558824485341162121989e-01L,
pS5 = -2.56165783329023486777386833928147375e-01L,
pS6 =  4.80718586374448793411019434585413855e-02L,
pS7 = -4.42523267167024279410230886239774718e-03L,
pS8 =  1.44551535183911458253205638280410064e-04L,
pS9 = -2.10558957916600254061591040482706179e-07L,
qS1 = -4.84690167848739751544716485245697428e+00L,
qS2 =  9.96619113536172610135016921140206980e+00L,
qS3 = -1.13177895428973036660836798461641458e+01L,
qS4 =  7.74004374389488266169304117714658761e+00L,
qS5 = -3.25871986053534084709023539900339905e+00L,
qS6 =  8.27830318881232209752469022352928864e-01L,
qS7 = -1.18768052702942805423330715206348004e-01L,
qS8 =  8.32600764660522313269101537926539470e-03L,
qS9 = -1.99407384882605586705979504567947007e-04L;

/*
 * atanl()
 */
const long double atanhi[] = {
	 4.63647609000806116214256231461214397e-01L,
	 7.85398163397448309615660845819875699e-01L,       
	 9.82793723247329067985710611014666038e-01L,       
	 1.57079632679489661923132169163975140e+00L,
};

const long double atanlo[] = {
	 4.89509642257333492668618435220297706e-36L,
	 2.16795253253094525619926100651083806e-35L,
	-2.31288434538183565909319952098066272e-35L,
	 4.33590506506189051239852201302167613e-35L,
};

const long double aT[] = {
	 3.33333333333333333333333333333333125e-01L,
	-1.99999999999999999999999999999180430e-01L,
	 1.42857142857142857142857142125269827e-01L,
	-1.11111111111111111111110834490810169e-01L,
	 9.09090909090909090908522355708623681e-02L,
	-7.69230769230769230696553844935357021e-02L,
	 6.66666666666666660390096773046256096e-02L,
	-5.88235294117646671706582985209643694e-02L,
	 5.26315789473666478515847092020327506e-02L,
	-4.76190476189855517021024424991436144e-02L,
	 4.34782608678695085948531993458097026e-02L,
	-3.99999999632663469330634215991142368e-02L,
	 3.70370363987423702891250829918659723e-02L,
	-3.44827496515048090726669907612335954e-02L,
	 3.22579620681420149871973710852268528e-02L,
	-3.03020767654269261041647570626778067e-02L,
	 2.85641979882534783223403715930946138e-02L,
	-2.69824879726738568189929461383741323e-02L,
	 2.54194698498808542954187110873675769e-02L,
	-2.35083879708189059926183138130183215e-02L,
	 2.04832358998165364349957325067131428e-02L,
	-1.54489555488544397858507248612362957e-02L,
	 8.64492360989278761493037861575248038e-03L,
	-2.58521121597609872727919154569765469e-03L,
};

const long double pi_lo = 8.67181013012378102479704402604335225e-35L;
