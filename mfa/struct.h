typedef struct POINT {uint16_t x; uint16_t y;};
typedef struct LINE {POINT a;POINT b;};
typedef struct HAND_POINTS {POINT a;POINT b; POINT e; POINT f;};
typedef struct HAND_SET{LINE Sec; HAND_POINTS Min; HAND_POINTS Hour;};

void copyPoint(POINT &dest, POINT src)
{
  dest.x = src.x;
  dest.y = src.y;
}

void copyHandSet(HAND_SET &dest,HAND_SET src)
{
  copyPoint(dest.Sec.a, src.Sec.a);
  copyPoint(dest.Sec.b, src.Sec.b);
  copyPoint(dest.Min.a, src.Min.a);
  copyPoint(dest.Min.b, src.Min.b);
  copyPoint(dest.Min.e, src.Min.e);
  copyPoint(dest.Min.f, src.Min.f);
  copyPoint(dest.Hour.a, src.Hour.a);
  copyPoint(dest.Hour.b, src.Hour.b);
  copyPoint(dest.Hour.e, src.Hour.e);
  copyPoint(dest.Hour.f, src.Hour.f);
}

void copyTME(tmElements_t &dest, tmElements_t src)
{
  dest.Second = src.Second;
  dest.Minute = src.Minute;
  dest.Hour = src.Hour;
  dest.Wday = src.Wday;
  dest.Day = src.Day;
  dest.Month = src.Month;
  dest.Year = src.Year;
}

