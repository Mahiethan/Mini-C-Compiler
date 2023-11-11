; ModuleID = 'mini-c'
source_filename = "mini-c"

define void @"lineTwo "(i32 %a, float %a1) {
entry:
  %a13 = alloca float, align 4
  %a2 = alloca i32, align 4
  store i32 %a, ptr %a2, align 4
  store float %a1, ptr %a13, align 4
  ret void
}

define float @asd() {
entry:
  %a3 = alloca i32, align 4
  %a = alloca float, align 4
  %a1 = load float, ptr %a, align 4
  %a2 = load float, ptr %a, align 4
  ret float %a2
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
  %aaa7 = load i1, ptr %aaa, align 1
  ret i1 %aaa7
}

define void @tester(i32 %v) {
entry:
  %v1 = alloca i32, align 4
  store i32 %v, ptr %v1, align 4
  ret void
}

define i32 @ret(float %h) {
entry:
  %o = alloca i32, align 4
  %h1 = alloca float, align 4
  store float %h, ptr %h1, align 4
  ret i32 0
}

define i32 @main() {
entry:
  %b = alloca i32, align 4
  %a = alloca i32, align 4
  %a1 = load i32, ptr %a, align 4
  call void @tester(i32 %a1)
  %b2 = load i32, ptr %b, align 4
  call void @tester(i32 %b2)
  %calltmp = call i32 @ret(float 9.000000e+01)
  %calltmp3 = call i32 @ret(float 9.000000e+01)
  ret i32 %calltmp3
}
