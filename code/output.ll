; ModuleID = 'mini-c'
source_filename = "mini-c"

@a = common global i32 0, align 4
@b = common global float 0.000000e+00, align 4
@c = common global i1 false, align 1
@o = common global i32 0, align 4

define float @asd() {
entry:
  %a = alloca float, align 4
  %load_temp = load float, ptr %a, align 4
  ret float %load_temp
}

define i1 @test(i32 %a, float %a1, i1 %a2) {
entry:
  %aaa = alloca i1, align 1
  %test = alloca float, align 4
  %a25 = alloca i1, align 1
  %a14 = alloca float, align 4
  %a3 = alloca i32, align 4
  store i32 %a, ptr %a3, align 4
  store float %a1, ptr %a14, align 4
  store i1 %a2, ptr %a25, align 1
  store float 5.000000e+00, ptr %test, align 4
  store i1 false, ptr %aaa, align 1
  %load_temp = load i1, ptr %aaa, align 1
  ret i1 %load_temp
}

define void @tester(i32 %v) {
entry:
  %v1 = alloca i32, align 4
  store i32 %v, ptr %v1, align 4
  ret void
}

define i32 @ret(float %h) {
entry:
  %h1 = alloca float, align 4
  store float %h, ptr %h1, align 4
  ret i32 0
}

define i32 @main() {
entry:
  %comboThree = alloca float, align 4
  %comboTwo = alloca float, align 4
  %combo = alloca float, align 4
  %not = alloca i32, align 4
  %unary = alloca float, align 4
  %mult = alloca float, align 4
  %div = alloca float, align 4
  %mod = alloca float, align 4
  %minus = alloca float, align 4
  %plus = alloca float, align 4
  %gt = alloca i1, align 1
  %ge = alloca i1, align 1
  %lt = alloca i1, align 1
  %le = alloca i1, align 1
  %neq = alloca i1, align 1
  %eq = alloca float, align 4
  %and = alloca i1, align 1
  %or2 = alloca float, align 4
  %or1 = alloca i32, align 4
  %or = alloca i1, align 1
  store i1 true, ptr %or, align 1
  store i1 true, ptr %or, align 1
  store i1 false, ptr %and, align 1
  store i1 false, ptr %and, align 1
  store float 1.000000e+00, ptr %eq, align 4
  store i1 false, ptr %neq, align 1
  store i1 true, ptr %le, align 1
  store i1 false, ptr %lt, align 1
  store i1 true, ptr %ge, align 1
  store i1 true, ptr %gt, align 1
  store float 1.000000e+00, ptr %plus, align 4
  store float -9.000000e+00, ptr %minus, align 4
  store float 9.200000e+01, ptr %mult, align 4
  store float 0.000000e+00, ptr %div, align 4
  store float 0.000000e+00, ptr %mod, align 4
  store float -1.000000e+02, ptr %unary, align 4
  %load_temp = load float, ptr %unary, align 4
  %fneg_temp = fneg float %load_temp
  store float %fneg_temp, ptr %combo, align 4
  store float %fneg_temp, ptr %unary, align 4
  store i32 -1, ptr %not, align 4
  store i32 0, ptr %not, align 4
  %load_temp1 = load float, ptr %unary, align 4
  %bool_cast = fcmp one float %load_temp1, 0.000000e+00
  %not_temp = xor i1 %bool_cast, true
  %load_temp2 = load i32, ptr %not, align 4
  %0 = zext i1 %not_temp to i32
  %add_tmp = add i32 %0, %load_temp2
  store i32 %add_tmp, ptr %not, align 4
  store float 0xC0AA9B2F80000000, ptr %combo, align 4
  store float 0x3F7EFBBD60000000, ptr %combo, align 4
  store float 0.000000e+00, ptr %combo, align 4
  store float 1.000000e+00, ptr %combo, align 4
  store float 0.000000e+00, ptr %comboTwo, align 4
  store float 1.000000e+00, ptr %comboThree, align 4
  store float 5.000000e+00, ptr %combo, align 4
  ret i32 0
}
