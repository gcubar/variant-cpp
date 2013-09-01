
#include <string>

#include "tr1.h"

#include TR1INCLUDE(memory)

#include "variant.h"
#include "gtest/gtest.h"

// number tests (int)
TEST(VariantTest, Integer)
{
    int iValue = 101;
    var vValue = iValue;

    int         tr1 = vValue.to<int>();
    int         tr2 = vValue;
    float       tr3 = vValue;
    double      tr4 = vValue;
    bool        tr5 = vValue;
    std::string tr6 = vValue;

    EXPECT_EQ       (iValue,    tr1);
    EXPECT_EQ       (iValue,    tr2);
    EXPECT_EQ       (101.0f,    tr3);
    EXPECT_EQ       (101.0f,    tr4);
    EXPECT_EQ       (true,      tr5);
    EXPECT_STREQ    ("101",     tr6.c_str());
}

// number tests (float)
TEST(VariantTest, Float)
{
    float fValue = 3.1415f;
    var vValue   = fValue;

    float       tr1 = vValue.to<float>();
    float       tr2 = vValue;
    int         tr3 = vValue;
    double      tr4 = vValue;
    bool        tr5 = vValue;
    std::string tr6 = vValue;

    EXPECT_FLOAT_EQ (fValue,    tr1);
    EXPECT_FLOAT_EQ (fValue,    tr2);
    EXPECT_EQ       (3, tr3);
    EXPECT_DOUBLE_EQ(3.1414999961853027,    tr4); // its impossible to exact result
    EXPECT_EQ       (true,      tr5);
    EXPECT_STREQ    ("3.1415",  tr6.c_str());
}

// number tests (double)
TEST(VariantTest, Double)
{
    double dValue = 3.1415;
    var vValue    = dValue;

    double      tr1 = vValue.to<double>();
    double      tr2 = vValue;
    int         tr3 = vValue;
    float       tr4 = vValue;
    bool        tr5 = vValue;
    std::string tr6 = vValue;

    EXPECT_DOUBLE_EQ(dValue,    tr1);
    EXPECT_DOUBLE_EQ(dValue,    tr2);
    EXPECT_EQ       (3,         tr3);
    EXPECT_FLOAT_EQ (3.1415f,   tr4);
    EXPECT_EQ       (true,      tr5);
    EXPECT_STREQ    ("3.1415",  tr6.c_str());
}

TEST(VariantTest, Boolean)
{
    bool bValue = true;
    var vValue  = bValue;

    bool        tr1 = vValue.to<bool>();
    bool        tr2 = vValue;
    int         tr3 = vValue;
    float       tr4 = vValue;
    double      tr5 = vValue;
    std::string tr6 = vValue;

    EXPECT_EQ       (bValue,    tr1);
    EXPECT_EQ       (bValue,    tr2);
    EXPECT_EQ       (1,         tr3);
    EXPECT_FLOAT_EQ (1.0f,      tr4);
    EXPECT_DOUBLE_EQ(1.0,       tr5);
    EXPECT_STREQ    ("true",    tr6.c_str());
}

TEST(VariantTest, Strings)
{
    std::string sValue = std::string("3.1415");
    var vValue         = sValue;

    std::string tr1 = vValue.to<std::string>();
    std::string tr2 = vValue;
    int         tr3 = vValue;
    float       tr4 = vValue;
    double      tr5 = vValue;
    bool        tr6 = vValue;

    EXPECT_STREQ    (sValue.c_str(), tr1.c_str());
    EXPECT_STREQ    (sValue.c_str(), tr2.c_str());
    EXPECT_EQ       (3,         tr3);
    EXPECT_FLOAT_EQ (3.1415f,   tr4);
    EXPECT_DOUBLE_EQ(3.1415,    tr5);
    EXPECT_EQ       (false,     tr6);
}

TEST(VariantTest, CustomClass)
{
    struct S
    {
        S() {}
        S(const int s) : _s(s) {}
        S(const S &s) : _s(s._s) {}

        int _s;
    };

    typedef std::tr1::shared_ptr<S> RS;

    S s = S(1);
    S *pS = new S(2);
    RS rs = RS(new S(3));

    var vValue = s;
    S rt = vValue;
    EXPECT_EQ       (s._s,      rt._s);

    vValue = pS;
    S *prt = vValue;
    EXPECT_EQ       (pS->_s,    prt->_s);
    prt->_s = 101;
    EXPECT_EQ       (pS->_s,    prt->_s);

    vValue = rs;
    RS rrs = vValue;
    EXPECT_EQ       (rs->_s,    rrs->_s);
    rrs->_s = 102;
    EXPECT_EQ       (rs->_s,    rrs->_s);

    delete pS;
}

TEST(VariantTest, MultipleAssigns)
{
    int iValue = 101;
    float fValue = 3.1415f;
    std::string sValue = "Multiple assigns";
    char *cValue = "Simple string";

    var vValue = iValue;
    int iCurrent = vValue;
    EXPECT_EQ       (iCurrent,  iValue);

    vValue = fValue;
    float fCurrent = vValue;
    EXPECT_EQ       (fCurrent,  fValue);

    vValue = sValue;
    std::string sCurrent = vValue;
    EXPECT_STREQ    (sCurrent.c_str(), sValue.c_str());

    vValue = cValue;
    char *cCurrent = vValue;
    EXPECT_STREQ    (cCurrent,  cValue);
}

TEST(VariantTest, Comparisons)
{
    int iValue = 101;
    var vValue = iValue;

    //TODO
    //bool bResult = iValue == vValue;
}
