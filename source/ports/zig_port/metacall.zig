fn metacall(comptime R: type, method: []const u8, args: anytype) !R {
	const T = @TypeOf(args);
	const fields = meta.fields(T);

	var args: [fields.len]MetaCall.Arg = undefined;
	var last: usize = 0;

	defer for (args[0..last]) |arg| metacall_value_destroy(arg);
	inline for (fields) |field, index| {
		last = index;
		args[index] = try metacall_value_create(field.field_type, @field(args, field.name));
	}

	const ret = metacallv_s(method, args.len, &args);
	defer metacall_value_destroy(ret);

	return metacall_value_to(R, ret);
}
