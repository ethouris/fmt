
#include <fstream> // for checking the file only
#include "fmt/sfmt.h"

#include "gtest-extra.h"



TEST(sfmt_test, formatting) {

  using std::string;

  EXPECT_EQ(fmt::sfmts(123.0, "#.0f"), "123.");
  EXPECT_EQ(fmt::sfmts(1.234, ".02f"), "1.23");
  EXPECT_EQ(fmt::sfmts(0.001, ".1g"), "0.001");

  // NOTE: differs to {fmt} method because the default format is %g (even for float).
  EXPECT_EQ(fmt::sfmts(1019666432.0f, ""), "1.01967e+09");
  EXPECT_EQ(fmt::sfmts(9.5, ".0e"), "1e+01");
  EXPECT_EQ(fmt::sfmts(1e-34, ".1e"), "1.0e-34");

  EXPECT_EQ(fmt::sfmts("str", ".2"), "st");
  EXPECT_EQ(fmt::sfmts("123456\xad", ".6"), "123456");

  EXPECT_EQ(fmt::sfmts(0.0, "9.1e"), "  0.0e+00");

  // Now the same, but using config flags
  EXPECT_EQ(fmt::sfmts(123.0, fmt::sfmc().alt().fixed().precision(0)), "123.");
  EXPECT_EQ(fmt::sfmts(1.234, fmt::sfmc().fixed().precision(2)), "1.23");
  EXPECT_EQ(fmt::sfmts(0.001, fmt::sfmc().general().precision(1)), "0.001");
  EXPECT_EQ(fmt::sfmts(1019666432.0f), "1.01967e+09");
  EXPECT_EQ(fmt::sfmts(9.5, fmt::sfmc().scientific().precision(0)), "1e+01");
  EXPECT_EQ(fmt::sfmts(1e-34, fmt::sfmc().scientific().precision(1)), "1.0e-34");

  EXPECT_EQ(fmt::sfmts("str", fmt::sfmc().precision(2)), "st");
  EXPECT_EQ(fmt::sfmts("123456\xad", fmt::sfmc().precision(6)), "123456");

  EXPECT_EQ(fmt::sfmts(0.0, fmt::sfmc().width(9).precision(1).exp()), "  0.0e+00");


  char ar[5] = "AR";
  char* par = ar;
  const char* pcar = ar;
  const void* pvar = ar;

  EXPECT_EQ(fmt::sfmts(ar), "AR");
  EXPECT_EQ(fmt::sfmts(par), "AR");
  EXPECT_EQ(fmt::sfmts(pcar), "AR");
  EXPECT_NE(fmt::sfmts(pvar), "AR");

  {
    fmt::ofilestream file ("TMPFILE");
    file << "NUMBER: " << fmt::sfmt(9.5, ".0e") << fmt::seol;
  }
  std::ifstream file ("TMPFILE");
  std::string line;
  std::getline(file, (line));
  remove("TMPFILE");

  EXPECT_EQ(line, "NUMBER: 1e+01");

  fmt::obufstream s;
  s << "NUMBER: " << fmt::sfmt(9.5, ".0e");

  std::string tmpbuf;
  s.copy_to(tmpbuf);

  EXPECT_EQ(tmpbuf, "NUMBER: 1e+01");
  EXPECT_EQ(s.str(), "NUMBER: 1e+01");

  std::string fragbuf;
  s.copy_to(fragbuf, 6);
  EXPECT_EQ(fragbuf, "NUMBER");

  s.clear();
  std::string loc = "local";

  s << "STRING:" << fmt::sfmt(loc, "10") << "mode";

  EXPECT_EQ(s.str(), "STRING:     localmode");

  // Multi-arg string building in place
  std::string joint = (fmt::obufstream() << "NUMBER: " << fmt::sfmt(9.5, ".0e")).str();

  EXPECT_EQ(joint, "NUMBER: 1e+01");
}
