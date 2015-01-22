#define        MAX_CALLS        200

int match(const char *mask, const char *name)
{
        int        calls=0, wild=0, q=0;
        const char *m=mask, *n=name, *ma=mask, *na=name;

        for(;;) {
                if (++calls > MAX_CALLS) return 1;
                if (*m == '*') {
                        while (*m == '*') ++m;
                        wild = 1;
                        ma = m;
                        na = n;
                }

                if (!*m) {
                        if (!*n) return 0;

                        for (--m; (m > mask) && (*m == '?'); --m) ;

                        if ((*m == '*') && (m > mask) &&
                            (m[-1] != '\\'))
                                return 0;
                        if (!wild)
                                return 1;
                        m = ma;
                } else if (!*n) {
                        while(*m == '*') ++m;
                        return (*m != 0);
                }
                if ((*m == '\\') && ((m[1] == '*') || (m[1] == '?'))) {
                        ++m;
                        q = 1;
                } else {
                        q = 0;
                }

                if ((tolower(*m) != tolower(*n)) && ((*m != '?') || q)) {
                        if (!wild) return 1;
                        m = ma;
                        n = ++na;
                } else {
                        if (*m) ++m
                        if (*n) ++n;
                }
        }
}

