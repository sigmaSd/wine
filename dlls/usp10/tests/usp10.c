/*
 * Tests for usp10 dll
 *
 * Copyright 2006 Jeff Latimer
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Notes:
 * Uniscribe allows for processing of complex scripts such as joining
 * and filtering characters and bi-directional text with custom line breaks.
 */

#include <assert.h>
#include <stdio.h>

#include <wine/test.h>
#include <winbase.h>
#include <wingdi.h>
#include <winuser.h>
#include <winerror.h>
#include <usp10.h>

static void test_ScriptItemIzeShapePlace(unsigned short pwOutGlyphs[256])
{
    HRESULT         hr;
    int             iMaxProps;
    const SCRIPT_PROPERTIES **ppSp;
    HWND            hwnd;
    HDC             hdc;

    int             cInChars;
    int             cMaxItems;
    SCRIPT_ITEM     pItem[255];
    int             pcItems;
    WCHAR           TestItem1[6] = {'T', 'e', 's', 't', 0x0166, 0}; 
    WCHAR           TestItem2[6] = {'T', 'e', 's', 't', 0x0166, 0}; 

    SCRIPT_CACHE    psc;
    int             cChars;
    int             cMaxGlyphs;
    unsigned short  pwOutGlyphs1[256];
    unsigned short  pwOutGlyphs2[256];
    unsigned short  pwLogClust[256];
    SCRIPT_VISATTR  psva[256];
    int             pcGlyphs;
    int             piAdvance[256];
    GOFFSET         pGoffset[256];
    ABC             pABC[256];
    int             cnt;

    /* We need a valid HDC to drive a lot of Script functions which requires the following    *
     * to set up for the tests.                                                               */
    hwnd = CreateWindowExA(0, "static", "", WS_POPUP, 0,0,100,100,
                           0, 0, 0, NULL);
    assert(hwnd != 0);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    hdc = GetDC(hwnd);                                      /* We now have a hdc             */
    ok( hdc != NULL, "HDC failed to be created %p\n", hdc);

    /* Start testing usp10 functions                                                         */
    /* This test determines that the pointer returned by ScriptGetProperties is valid
     * by checking a known value in the table                                                */
    hr = ScriptGetProperties(&ppSp, &iMaxProps);
    trace("number of script properties %d\n", iMaxProps);
    ok (iMaxProps > 0, "Number of scripts returned should not be 0\n"); 
    if  (iMaxProps > 0)
         ok( ppSp[5]->langid == 9, "Langid[5] not = to 9\n"); /* Check a known value to ensure   */
                                                              /* ptrs work                       */


    /* This set of tests are to check that the various edits in ScriptIemize work           */
    cInChars = 5;                                        /* Length of test without NULL     */
    cMaxItems = 1;                                       /* Check threshold value           */
    hr = ScriptItemize(TestItem1, cInChars, cMaxItems, NULL, NULL, pItem, &pcItems);
    ok (hr == E_INVALIDARG, "ScriptItemize should return E_INVALIDARG if cMaxItems < 2.  Was %d\n",
        cMaxItems);
    cInChars = 5;
    cMaxItems = 255;
    hr = ScriptItemize(NULL, cInChars, cMaxItems, NULL, NULL, pItem, &pcItems);
    ok (hr == E_INVALIDARG, "ScriptItemize should return E_INVALIDARG if pwcInChars is NULL\n");

    cInChars = 5;
    cMaxItems = 255;
    hr = ScriptItemize(TestItem1, 0, cMaxItems, NULL, NULL, pItem, &pcItems);
    ok (hr == E_INVALIDARG, "ScriptItemize should return E_INVALIDARG if cInChars is 0\n");

    cInChars = 5;
    cMaxItems = 255;
    hr = ScriptItemize(TestItem1, cInChars, cMaxItems, NULL, NULL, NULL, &pcItems);
    ok (hr == E_INVALIDARG, "ScriptItemize should return E_INVALIDARG if pItems is NULL\n");

    /* This is a valid test that will cause parsing to take place                             */
    cInChars = 5;
    cMaxItems = 255;
    hr = ScriptItemize(TestItem1, cInChars, cMaxItems, NULL, NULL, pItem, &pcItems);
    ok (hr == 0, "ScriptItemize should return 0, returned %08x\n", (unsigned int) hr);
    /*  This test is for the interim operation of ScriptItemize where only one SCRIPT_ITEM is *
     *  returned.                                                                             */
    ok (pcItems > 0, "The number of SCRIPT_ITEMS should be greater than 0\n");
    if (pcItems > 0)
        ok (pItem[0].iCharPos == 0 && pItem[1].iCharPos == cInChars,
            "Start pos not = 0 (%d) or end pos not = %d (%d)\n",
            pItem[0].iCharPos, cInChars, pItem[1].iCharPos);

    /* It would appear that we have a valid SCRIPT_ANALYSIS and can continue
     * ie. ScriptItemize has succeeded and that pItem has been set                            */
    cInChars = 5;
    cMaxItems = 255;
    if (hr == 0) {
        psc = NULL;                                   /* must be null on first call           */
        cChars = cInChars;
        cMaxGlyphs = cInChars;
        hr = ScriptShape(NULL, &psc, TestItem1, cChars,
                         cMaxGlyphs, &pItem[0].a,
                         pwOutGlyphs1, pwLogClust, psva, &pcGlyphs);
        ok (hr == E_PENDING, "If psc is NULL (%08x) the E_PENDING should be returned\n",
                      (unsigned int) hr);
        cMaxGlyphs = 4;
        hr = ScriptShape(hdc, &psc, TestItem1, cChars,
                         cMaxGlyphs, &pItem[0].a,
                         pwOutGlyphs1, pwLogClust, psva, &pcGlyphs);
        ok (hr == E_OUTOFMEMORY, "If not enough output area cChars (%d) is > than CMaxGlyphs "
                                 "(%d) but not E_OUTOFMEMORY\n",
                                 cChars, cMaxGlyphs);
        cMaxGlyphs = 256;
        hr = ScriptShape(hdc, &psc, TestItem1, cChars,
                         cMaxGlyphs, &pItem[0].a,
                         pwOutGlyphs1, pwLogClust, psva, &pcGlyphs);
        ok (hr == 0, "ScriptShape should return 0 not (%08x)\n", (unsigned int) hr);
        ok (psc != NULL, "psc should not be null and have SCRIPT_CACHE buffer address\n");
        ok (pcGlyphs == cChars, "Chars in (%d) should equal Glyphs out (%d)\n", cChars, pcGlyphs);
        if (hr ==0) {
            hr = ScriptPlace(NULL, &psc, pwOutGlyphs1, pcGlyphs, psva, &pItem[0].a, piAdvance,
                             pGoffset, pABC);
            ok (hr == 0, "Should return 0 not (%08x)\n", (unsigned int) hr);
            for (cnt=0; cnt < pcGlyphs; cnt++)
                pwOutGlyphs[cnt] = pwOutGlyphs1[cnt];                 /* Send to next function */
        }

        /* This test will check to make sure that SCRIPT_CACHE is reused and that not translation   *
         * takes place if fNoGlyphIndex is set.                                                     */

        cInChars = 5;
        cMaxItems = 255;
        hr = ScriptItemize(TestItem2, cInChars, cMaxItems, NULL, NULL, pItem, &pcItems);
        ok (hr == 0, "ScriptItemize should return 0, returned %08x\n", (unsigned int) hr);
        /*  This test is for the intertrim operation of ScriptItemize where only one SCRIPT_ITEM is *
         *  returned.                                                                               */
        ok (pItem[0].iCharPos == 0 && pItem[1].iCharPos == cInChars,
                            "Start pos not = 0 (%d) or end pos not = %d (%d)\n",
                             pItem[0].iCharPos, cInChars, pItem[1].iCharPos);
        /* It would appear that we have a valid SCRIPT_ANALYSIS and can continue                    */
        if (hr == 0) {
             cChars = cInChars;
             cMaxGlyphs = 256;
             pItem[0].a.fNoGlyphIndex = 1;                /* say no translate                     */
             hr = ScriptShape(NULL, &psc, TestItem2, cChars,
                              cMaxGlyphs, &pItem[0].a,
                              pwOutGlyphs2, pwLogClust, psva, &pcGlyphs);
             ok (hr != E_PENDING, "If psc should not be NULL (%08x) and the E_PENDING should be returned\n",
                (unsigned int) hr);
             ok (hr == 0, "ScriptShape should return 0 not (%08x)\n", (unsigned int) hr);
             ok (psc != NULL, "psc should not be null and have SCRIPT_CACHE buffer address\n");
             ok (pcGlyphs == cChars, "Chars in (%d) should equal Glyphs out (%d)\n", cChars, pcGlyphs);
             for (cnt=0; cnt < cChars && TestItem2[cnt] == pwOutGlyphs2[cnt]; cnt++) {}
             ok (cnt == cChars, "Translation to place when told not to. WCHAR %d - %04x != %04x\n",
                           cnt, TestItem2[cnt], pwOutGlyphs2[cnt]);
             if (hr ==0) {
                 hr = ScriptPlace(NULL, &psc, pwOutGlyphs2, pcGlyphs, psva, &pItem[0].a, piAdvance,
                                  pGoffset, pABC);
                 ok (hr == 0, "ScriptPlace should return 0 not (%08x)\n", (unsigned int) hr);
             }
        }
        hr = ScriptFreeCache( &psc);
        ok (!psc, "psc is not null after ScriptFreeCache\n");


    }
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
}

void test_ScriptGetCMap(unsigned short pwOutGlyphs[256])
{
    HRESULT         hr;
    HWND            hwnd;
    HDC             hdc;
    SCRIPT_CACHE    psc = NULL;
    int             cInChars;
    int             cChars;
    unsigned short  pwOutGlyphs3[256];
    WCHAR           TestItem1[6] = {'T', 'e', 's', 't', 0x0166, 0}; 
    DWORD           dwFlags;
    int             cnt;

    /* We need a valid HDC to drive a lot of Script functions which requires the following    *
     * to set up for the tests.                                                               */
    hwnd = CreateWindowExA(0, "static", "", WS_POPUP, 0,0,100,100,
                           0, 0, 0, NULL);
    assert(hwnd != 0);

    hdc = GetDC(hwnd);
    ok( hdc != NULL, "HDC failed to be created %p\n", hdc);

    /*  Check to make sure that SCRIPT_CACHE gets allocated ok                     */
    dwFlags = 0;
    cInChars = cChars = 5;
    /* Some sanity checks for ScriptGetCMap */

    hr = ScriptGetCMap(NULL, NULL, NULL, 0, 0, NULL);
    ok( hr == E_INVALIDARG, "(NULL,NULL,NULL,0,0,NULL), "
                            "expected E_INVALIDARG, got %08x\n", (unsigned int)hr);

    hr = ScriptGetCMap(NULL, NULL, TestItem1, cInChars, dwFlags, pwOutGlyphs3);
    ok( hr == E_INVALIDARG, "(NULL,NULL,TestItem1, cInChars, dwFlags, pwOutGlyphs3), "
                            "expected E_INVALIDARG, got %08x\n", (unsigned int)hr);

    /* Set psc to NULL, to be able to check if a pointer is returned in psc */
    psc = NULL;
    hr = ScriptGetCMap(NULL, &psc, NULL, 0, 0, NULL);
    ok( hr == E_INVALIDARG, "(NULL,&psc,NULL,0,0NULL), expected E_INVALIDARG, "
                            "got %08x\n", (unsigned int)hr);
    ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);

    /* Set psc to NULL, to be able to check if a pointer is returned in psc */
    psc = NULL;
    hr = ScriptGetCMap(NULL, &psc, TestItem1, cInChars, dwFlags, pwOutGlyphs3);
    ok( hr == E_PENDING, "(NULL,&psc,), expected E_PENDING, got %08x\n", (unsigned int)hr);
    ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);
    /*  Check to see if the results are the same as those returned by ScriptShape  */
    hr = ScriptGetCMap(hdc, &psc, TestItem1, cInChars, dwFlags, pwOutGlyphs3);
    ok (hr == 0, "ScriptGetCMap should return 0 not (%08x)\n", (unsigned int) hr);
    ok (psc != NULL, "psc should not be null and have SCRIPT_CACHE buffer address\n");
    for (cnt=0; cnt < cChars && pwOutGlyphs[cnt] == pwOutGlyphs3[cnt]; cnt++) {}
    ok (cnt == cInChars, "Translation not correct. WCHAR %d - %04x != %04x\n",
                         cnt, pwOutGlyphs[cnt], pwOutGlyphs3[cnt]);
        
    hr = ScriptFreeCache( &psc);
    ok (!psc, "psc is not null after ScriptFreeCache\n");
}

void test_ScriptGetFontProperties(void)
{
    HRESULT         hr;
    HDC             hdc;
    HWND            hwnd;
    SCRIPT_CACHE    psc,old_psc;
    SCRIPT_FONTPROPERTIES sfp;

    /* Only do the bare minumum to get a valid hdc */
    hwnd = CreateWindowExA(0, "static", "", WS_POPUP, 0,0,100,100,0, 0, 0, NULL);
    assert(hwnd != 0);

    hdc = GetDC(hwnd);
    ok( hdc != NULL, "HDC failed to be created %p\n", hdc);

    /* Some sanity checks for ScriptGetFontProperties */

    hr = ScriptGetFontProperties(NULL,NULL,NULL);
    ok( hr == E_INVALIDARG, "(NULL,NULL,NULL), expected E_INVALIDARG, got %08x\n", (unsigned int)hr);

    hr = ScriptGetFontProperties(NULL,NULL,&sfp);
    ok( hr == E_INVALIDARG, "(NULL,NULL,&sfp), expected E_INVALIDARG, got %08x\n", (unsigned int)hr);

    /* Set psc to NULL, to be able to check if a pointer is returned in psc */
    psc = NULL;
    hr = ScriptGetFontProperties(NULL,&psc,NULL);
    ok( hr == E_INVALIDARG, "(NULL,&psc,NULL), expected E_INVALIDARG, got %08x\n", (unsigned int)hr);
    ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);

    /* Set psc to NULL, to be able to check if a pointer is returned in psc */
    psc = NULL;
    hr = ScriptGetFontProperties(NULL,&psc,&sfp);
    ok( hr == E_PENDING, "(NULL,&psc,&sfp), expected E_PENDING, got %08x\n", (unsigned int)hr);
    ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);

    hr = ScriptGetFontProperties(hdc,NULL,NULL);
    ok( hr == E_INVALIDARG, "(hdc,NULL,NULL), expected E_INVALIDARG, got %08x\n", (unsigned int)hr);

    hr = ScriptGetFontProperties(hdc,NULL,&sfp);
    ok( hr == E_INVALIDARG, "(hdc,NULL,&sfp), expected E_INVALIDARG, got %08x\n", (unsigned int)hr);

    /* Set psc to NULL, to be able to check if a pointer is returned in psc */
    psc = NULL;
    hr = ScriptGetFontProperties(hdc,&psc,NULL);
    ok( hr == E_INVALIDARG, "(hdc,&psc,NULL), expected E_INVALIDARG, got %08x\n", (unsigned int)hr);
    ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);

    /* Pass an uninitialized sfp */
    psc = NULL;
    hr = ScriptGetFontProperties(hdc,&psc,&sfp);
    ok( hr == E_INVALIDARG, "(hdc,&psc,&sfp) partly uninitialized, expected E_INVALIDARG, got %08x\n", (unsigned int)hr);
    ok( psc != NULL, "Expected a pointer in psc, got NULL\n");
    ScriptFreeCache(&psc);
    ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);

    /* Give it the correct cBytes, we don't care about what's coming back */
    sfp.cBytes = sizeof(SCRIPT_FONTPROPERTIES);
    psc = NULL;
    hr = ScriptGetFontProperties(hdc,&psc,&sfp);
    ok( hr == S_OK, "(hdc,&psc,&sfp) partly initialized, expected S_OK, got %08x\n", (unsigned int)hr);
    ok( psc != NULL, "Expected a pointer in psc, got NULL\n");

    /* Save the psc pointer */
    old_psc = psc;
    /* Now a NULL hdc again */
    hr = ScriptGetFontProperties(NULL,&psc,&sfp);
    ok( hr == S_OK, "(NULL,&psc,&sfp), expected S_OK, got %08x\n", (unsigned int)hr);
    ok( psc == old_psc, "Expected psc not to be changed, was %p is now %p\n", old_psc, psc);
    ScriptFreeCache(&psc);
    ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);

    /* Cleanup */
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
}

void test_ScriptTextOut(void)
{
    HRESULT         hr;
    HWND            hwnd;
    HDC             hdc;

    int             cInChars;
    int             cMaxItems;
    SCRIPT_ITEM     pItem[255];
    int             pcItems;
    WCHAR           TestItem1[6] = {'T', 'e', 's', 't', 0x0166, 0}; 

    SCRIPT_CACHE    psc;
    int             cChars;
    int             cMaxGlyphs;
    unsigned short  pwOutGlyphs1[256];
    unsigned short  pwLogClust[256];
    SCRIPT_VISATTR  psva[256];
    int             pcGlyphs;
    int             piAdvance[256];
    GOFFSET         pGoffset[256];
    ABC             pABC[256];
    RECT            rect;

    /* We need a valid HDC to drive a lot of Script functions which requires the following    *
     * to set up for the tests.                                                               */
    hwnd = CreateWindowExA(0, "static", "", WS_POPUP, 0,0,100,100,
                           0, 0, 0, NULL);
    assert(hwnd != 0);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    hdc = GetDC(hwnd);                                      /* We now have a hdc             */
    ok( hdc != NULL, "HDC failed to be created %p\n", hdc);

    /* This is a valid test that will cause parsing to take place                             */
    cInChars = 5;
    cMaxItems = 255;
    hr = ScriptItemize(TestItem1, cInChars, cMaxItems, NULL, NULL, pItem, &pcItems);
    ok (hr == 0, "ScriptItemize should return 0, returned %08x\n", (unsigned int) hr);
    /*  This test is for the interim operation of ScriptItemize where only one SCRIPT_ITEM is *
     *  returned.                                                                             */
    ok (pcItems > 0, "The number of SCRIPT_ITEMS should be greater than 0\n");
    if (pcItems > 0)
        ok (pItem[0].iCharPos == 0 && pItem[1].iCharPos == cInChars,
            "Start pos not = 0 (%d) or end pos not = %d (%d)\n",
            pItem[0].iCharPos, cInChars, pItem[1].iCharPos);

    /* It would appear that we have a valid SCRIPT_ANALYSIS and can continue
     * ie. ScriptItemize has succeeded and that pItem has been set                            */
    cInChars = 5;
    cMaxItems = 255;
    if (hr == 0) {
        psc = NULL;                                   /* must be null on first call           */
        cChars = cInChars;
        cMaxGlyphs = cInChars;
        cMaxGlyphs = 256;
        hr = ScriptShape(hdc, &psc, TestItem1, cChars,
                         cMaxGlyphs, &pItem[0].a,
                         pwOutGlyphs1, pwLogClust, psva, &pcGlyphs);
        ok (hr == 0, "ScriptShape should return 0 not (%08x)\n", (unsigned int) hr);
        ok (psc != NULL, "psc should not be null and have SCRIPT_CACHE buffer address\n");
        ok (pcGlyphs == cChars, "Chars in (%d) should equal Glyphs out (%d)\n", cChars, pcGlyphs);
        if (hr ==0) {
            hr = ScriptPlace(NULL, &psc, pwOutGlyphs1, pcGlyphs, psva, &pItem[0].a, piAdvance,
                             pGoffset, pABC);
            ok (hr == 0, "Should return 0 not (%08x)\n", (unsigned int) hr);
            ScriptFreeCache(&psc);              /* Get rid of psc for next test set */
            ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);

            hr = ScriptTextOut(NULL, NULL, 0, 0, 0, NULL, NULL, NULL, 0, NULL, 0, NULL, NULL, NULL);
            ok (hr == E_INVALIDARG, "Should return 0 not (%08x)\n", (unsigned int) hr);

            hr = ScriptTextOut(NULL, NULL, 0, 0, 0, NULL, &pItem[0].a, NULL, 0, pwOutGlyphs1, pcGlyphs,
                               piAdvance, NULL, pGoffset);
            ok( hr == E_INVALIDARG, "(NULL,NULL,TestItem1, cInChars, dwFlags, pwOutGlyphs3), "
                                    "expected E_INVALIDARG, got %08x\n", (unsigned int)hr);

            /* Set psc to NULL, to be able to check if a pointer is returned in psc */
            psc = NULL;
            hr = ScriptTextOut(NULL, &psc, 0, 0, 0, NULL, NULL, NULL, 0, NULL, 0,
                               NULL, NULL, NULL);
            ok( hr == E_INVALIDARG, "(NULL,&psc,NULL,0,0,0,NULL,), expected E_INVALIDARG, "
                                    "got %08x\n", (unsigned int)hr);
            ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);

            /* Set psc to NULL, to be able to check if a pointer is returned in psc */
            psc = NULL;
            hr = ScriptTextOut(NULL, &psc, 0, 0, 0, NULL, &pItem[0].a, NULL, 0, pwOutGlyphs1, pcGlyphs,
                               piAdvance, NULL, pGoffset);
            ok( hr == E_PENDING, "(NULL,&psc,), expected E_PENDING, got %08x\n", (unsigned int)hr);
            ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);

            /* Se that is gets a psc and that returns 0 status */
            hr = ScriptTextOut(hdc, &psc, 0, 0, 0, NULL, &pItem[0].a, NULL, 0, pwOutGlyphs1, pcGlyphs,
                               piAdvance, NULL, pGoffset);
            ok (hr == 0, "ScriptTextOut should return 0 not (%08x)\n", (unsigned int) hr);
            ok (psc != NULL, "psc should not be null and have SCRIPT_CACHE buffer address\n");

            /* Test Rect Rgn is acceptable and that it works without hdc */
            rect.top = 10;
            rect.bottom = 20;
            rect.left = 10;
            rect.right = 40;
            hr = ScriptTextOut(NULL, &psc, 0, 0, 0, &rect, &pItem[0].a, NULL, 0, pwOutGlyphs1, pcGlyphs,
                               piAdvance, NULL, pGoffset);
            ok (hr == 0, "ScriptTextOut should return 0 not (%08x)\n", (unsigned int) hr);
            ok (psc != NULL, "psc should not be null and have SCRIPT_CACHE buffer address\n");

            /* Clean up and go   */
            ScriptFreeCache(&psc);
            ok( psc == NULL, "Expected psc to be NULL, got %p\n", psc);
        }
    }
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
}

START_TEST(usp10)
{
    unsigned short  pwOutGlyphs[256];

    test_ScriptItemIzeShapePlace(pwOutGlyphs);
    test_ScriptGetCMap(pwOutGlyphs);
    test_ScriptGetFontProperties();
    test_ScriptTextOut();
}
