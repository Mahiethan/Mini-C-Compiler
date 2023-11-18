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
  %ty1 = alloca float, align 4
  %ty = alloca i32, align 4
  %combo = alloca float, align 4
  %load_global_temp = load i32, ptr @o, align 4
  store i32 %load_global_temp, ptr %ty, align 4
  %load_temp = load i32, ptr %ty, align 4
  %0 = sitofp i32 %load_temp to float
  %feq_tmp = fcmp oeq float %0, 0x3FF19999A0000000
  %if_cond = icmp ne i1 %feq_tmp, false
  br i1 %if_cond, label %then, label %end

then:                                             ; preds = %entry
  store float 1.000000e+01, ptr %ty1, align 4
  store i32 4, ptr @o, align 4
  br label %end

end:                                              ; preds = %then, %entry
  %load_global_temp2 = load i32, ptr @o, align 4
  %sub_tmp = sub i32 %load_global_temp2, -9
  %itof_cast = sitofp i32 %sub_tmp to float
  store float %itof_cast, ptr %combo, align 4
  store float 5.000000e+00, ptr %combo, align 4
  ret i32 0
}
