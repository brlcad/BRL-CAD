/* Generated by re2c */
#line 1 "condition_03.cg.re"

#line 5 "<stdout>"
{
	YYCTYPE yych;
	static void *yyctable[3] = {
		&&yyc_0,
		&&yyc_r1,
		&&yyc_r2,
	};
	goto *yyctable[YYGETCONDITION()];
/* *********************************** */
yyc_0:

#line 3 "condition_03.cg.re"
	{ return NULL; }
#line 19 "<stdout>"
/* *********************************** */
yyc_r1:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '2') {
		if (yych <= '0') goto yy5;
		if (yych <= '1') goto yy8;
		goto yy6;
	} else {
		if (yych <= '`') goto yy5;
		if (yych <= 'a') goto yy10;
		if (yych <= 'b') goto yy12;
	}
yy5:
yy6:
	++YYCURSOR;
#line 5 "condition_03.cg.re"
	{ return "2"; }
#line 38 "<stdout>"
yy8:
	++YYCURSOR;
#line 4 "condition_03.cg.re"
	{ return "1"; }
#line 43 "<stdout>"
yy10:
	++YYCURSOR;
#line 6 "condition_03.cg.re"
	{ return "a"; }
#line 48 "<stdout>"
yy12:
	++YYCURSOR;
#line 7 "condition_03.cg.re"
	{ return "b"; }
#line 53 "<stdout>"
/* *********************************** */
yyc_r2:
	if (YYLIMIT <= YYCURSOR) YYFILL(1);
	yych = *YYCURSOR;
	if (yych <= '2') {
		if (yych <= '0') goto yy16;
		if (yych <= '1') goto yy19;
		goto yy17;
	} else {
		if (yych == 'b') goto yy21;
	}
yy16:
yy17:
	++YYCURSOR;
#line 5 "condition_03.cg.re"
	{ return "2"; }
#line 70 "<stdout>"
yy19:
	++YYCURSOR;
#line 4 "condition_03.cg.re"
	{ return "1"; }
#line 75 "<stdout>"
yy21:
	++YYCURSOR;
#line 7 "condition_03.cg.re"
	{ return "b"; }
#line 80 "<stdout>"
}
#line 9 "condition_03.cg.re"

