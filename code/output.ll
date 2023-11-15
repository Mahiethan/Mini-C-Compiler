; ModuleID = 'mini-c'
source_filename = "mini-c"

@a = common global i32 0, align 4
@b = common global float 0.000000e+00, align 4
@c = common global i1 false, align 1
@o = common global i32 0, align 4

define float @asd() {
entry:
  %a = alloca float, align 4
  %a1 = load float, ptr %a, align 4
  ret float %a1
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
  %aaa6 = load i1, ptr %aaa, align 1
  ret i1 %aaa6
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
  %combo = alloca float, align 4
  %not = alloca float, align 4
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
  %or3 = load i1, ptr %or, align 1
  store i1 true, ptr %or, align 1
  %and4 = load i1, ptr %and, align 1
  store i1 false, ptr %and, align 1
  %eq5 = load float, ptr %eq, align 4
  store float 1.000000e+00, ptr %eq, align 4
  %neq6 = load i1, ptr %neq, align 1
  store i1 true, ptr %neq, align 1
  %le7 = load i1, ptr %le, align 1
  store i1 false, ptr %le, align 1
  %lt8 = load i1, ptr %lt, align 1
  store i1 false, ptr %lt, align 1
  %ge9 = load i1, ptr %ge, align 1
  store i1 true, ptr %ge, align 1
  %gt10 = load i1, ptr %gt, align 1
  store i1 true, ptr %gt, align 1
  %plus11 = load float, ptr %plus, align 4
  store float 1.000000e+00, ptr %plus, align 4
  %minus12 = load float, ptr %minus, align 4
  store float 1.000000e+00, ptr %minus, align 4
  %mult13 = load float, ptr %mult, align 4
  store float 0.000000e+00, ptr %mult, align 4
  %div14 = load float, ptr %div, align 4
  store float 0x40590AE140000000, ptr %div, align 4
  %div15 = load float, ptr %div, align 4
  store float 0.000000e+00, ptr %div, align 4
  %mod16 = load float, ptr %mod, align 4
  store float 0.000000e+00, ptr %mod, align 4
  %unary17 = load float, ptr %unary, align 4
  store float -1.000000e+02, ptr %unary, align 4
  %not18 = load float, ptr %not, align 4
  store float 0.000000e+00, ptr %not, align 4
  %combo19 = load float, ptr %combo, align 4
  store float 0xC11927C240000000, ptr %combo, align 4
  %combo20 = load float, ptr %combo, align 4
  store float 1.000000e+00, ptr %combo, align 4
  %calltmp = call i32 @ret(i1 true)
  ret i32 %calltmp
}
