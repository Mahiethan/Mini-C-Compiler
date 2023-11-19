; ModuleID = 'mini-c'
source_filename = "mini-c"

@test = common global i32 0, align 4
@f = common global float 0.000000e+00, align 4
@b = common global i1 false, align 1

declare i32 @print_int(i32)

define i32 @While(i32 %n) {
entry:
  %result = alloca i32, align 4
  %n1 = alloca i32, align 4
  store i32 %n, ptr %n1, align 4
  store i32 12, ptr @test, align 4
  store i32 0, ptr %result, align 4
  %load_global_arg = load i32, ptr @test, align 4
  %call_tmp = call i32 @print_int(i32 %load_global_arg)
  br label %while_cond

while_cond:                                       ; preds = %while_body, %entry
  %load_temp = load i32, ptr %result, align 4
  %lt_tmp = icmp slt i32 %load_temp, 10
  %if_cond = icmp ne i1 %lt_tmp, false
  br i1 %if_cond, label %while_body, label %while_end

while_body:                                       ; preds = %while_cond
  %load_temp2 = load i32, ptr %result, align 4
  %add_tmp = add i32 %load_temp2, 1
  store i32 %add_tmp, ptr %result, align 4
  br label %while_cond

while_end:                                        ; preds = %while_cond
  %load_temp3 = load i32, ptr %result, align 4
  ret i32 %load_temp3
}
