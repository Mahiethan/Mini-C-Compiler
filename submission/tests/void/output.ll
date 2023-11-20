; ModuleID = 'mini-c'
source_filename = "mini-c"

declare i32 @print_int(i32)

define void @Void() {
entry:
  %result = alloca i32, align 4
  store i32 0, ptr %result, align 4
  %load_arg = load i32, ptr %result, align 4
  %call_tmp = call i32 @print_int(i32 %load_arg)
  br label %while_cond

while_cond:                                       ; preds = %while_body, %entry
  %load_temp = load i32, ptr %result, align 4
  %lt_tmp = icmp slt i32 %load_temp, 10
  %if_cond = icmp ne i1 %lt_tmp, false
  br i1 %if_cond, label %while_body, label %while_end

while_body:                                       ; preds = %while_cond
  %load_temp1 = load i32, ptr %result, align 4
  %add_tmp = add i32 %load_temp1, 1
  store i32 %add_tmp, ptr %result, align 4
  %load_arg2 = load i32, ptr %result, align 4
  %call_tmp3 = call i32 @print_int(i32 %load_arg2)
  br label %while_cond

while_end:                                        ; preds = %while_cond
  ret void
}
