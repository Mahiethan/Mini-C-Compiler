; ModuleID = 'mini-c'
source_filename = "mini-c"

@a = common global i32 0, align 4
@b = common global float 0.000000e+00, align 4
@c = common global i1 false, align 1
@o = common global i32 0, align 4

declare void @lineTwo(i32, float)

define float @asd() {
entry:
  %a = alloca float, align 4
  %load_temp = load float, ptr %a, align 4
  ret float %load_temp
}

define i1 @test(i32 %a, i32 %b, i1 %c) {
entry:
  %aaa = alloca i1, align 1
  %test = alloca float, align 4
  %c3 = alloca i1, align 1
  %b2 = alloca i32, align 4
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  store i32 %b, ptr %b2, align 4
  store i1 %c, ptr %c3, align 1
  store float 5.000000e+00, ptr %test, align 4
  store i1 false, ptr %aaa, align 1
  %load_temp = load i32, ptr %a1, align 4
  %load_temp4 = load i32, ptr %b2, align 4
  %add_tmp = add i32 %load_temp, %load_temp4
  %load_temp5 = load i1, ptr %c3, align 1
  %0 = zext i1 %load_temp5 to i32
  %add_tmp6 = add i32 %add_tmp, %0
  %itof_cast = sitofp i32 %add_tmp6 to float
  store float %itof_cast, ptr %test, align 4
  %load_temp7 = load i1, ptr %aaa, align 1
  ret i1 %load_temp7
}

define i32 @addition(i32 %n, i32 %m) {
entry:
  %result = alloca i32, align 4
  %m2 = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  store i32 %m, ptr %m2, align 4
  %load_temp = load i32, ptr %n1, align 4
  %load_temp3 = load i32, ptr %m2, align 4
  %add_tmp = add i32 %load_temp, %load_temp3
  store i32 %add_tmp, ptr %result, align 4
  %load_temp4 = load i32, ptr %result, align 4
  ret i32 %load_temp4
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
  %ty = alloca i32, align 4
  %combo = alloca float, align 4
  %load_global_temp = load i32, ptr @o, align 4
  store i32 %load_global_temp, ptr %ty, align 4
  %load_global_temp1 = load i32, ptr @o, align 4
  %eq_tmp = icmp eq i32 %load_global_temp1, 10
  %if_cond = icmp ne i1 %eq_tmp, false
  br i1 %if_cond, label %if_then, label %if_else

if_then:                                          ; preds = %entry
  store i32 1, ptr %ty, align 4
  br label %if_end

if_else:                                          ; preds = %entry
  store i32 7, ptr %ty, align 4
  br label %if_end

if_end:                                           ; preds = %if_else, %if_then
  %load_global_temp2 = load i32, ptr @o, align 4
  %sub_tmp = sub i32 %load_global_temp2, -9
  %itof_cast = sitofp i32 %sub_tmp to float
  store float %itof_cast, ptr %combo, align 4
  store float 5.000000e+00, ptr %combo, align 4
  %load_global_arg = load i32, ptr @o, align 4
  %load_arg = load float, ptr %combo, align 4
  call void @lineTwo(i32 %load_global_arg, float %load_arg)
  ret i32 0
}
