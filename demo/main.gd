extends Control

# Метка сборки: если на экране другой текст — в APK старый скрипт
const BUILD := "v4-diag"

@onready var texture_rect: TextureRect = $TextureRect
@onready var status_label: Label = $StatusLabel

var doom
var touch_ui: TouchUI


func _show(text: String, hide_after := 0.0) -> void:
	status_label.text = text
	status_label.autowrap_mode = TextServer.AUTOWRAP_WORD_SMART
	status_label.visible = true
	if hide_after > 0.0:
		get_tree().create_timer(hide_after).timeout.connect(func() -> void:
			status_label.visible = false
		)


func _ready() -> void:
	var old := get_node_or_null("TouchControls")
	if old:
		old.queue_free()

	texture_rect.set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
	texture_rect.expand_mode = TextureRect.EXPAND_IGNORE_SIZE
	texture_rect.stretch_mode = TextureRect.STRETCH_KEEP_ASPECT_CENTERED
	texture_rect.mouse_filter = Control.MOUSE_FILTER_IGNORE

	touch_ui = TouchUI.new()
	add_child(touch_ui)
	touch_ui.key_changed.connect(_on_key_changed)

	# ========== Диагностика GDExtension ==========
	var ext_path := "res://addons/doom/doom.gdextension"
	var lines: PackedStringArray = []
	lines.append("BUILD %s" % BUILD)
	lines.append("arch=%s" % Engine.get_architecture_name())
	lines.append("ext_file=%s" % FileAccess.file_exists(ext_path))

	var so_debug := "res://addons/doom/bin/libdoom.android.template_debug.arm64.so"
	var so_release := "res://addons/doom/bin/libdoom.android.template_release.arm64.so"
	lines.append("so_debug=%s" % FileAccess.file_exists(so_debug))
	lines.append("so_release=%s" % FileAccess.file_exists(so_release))

	if ClassDB.class_exists("GDExtensionManager"):
		var loaded = GDExtensionManager.get_loaded_extensions()
		lines.append("loaded=%s" % str(loaded))
		# is_extension_loaded есть в 4.x
		if GDExtensionManager.has_method("is_extension_loaded"):
			lines.append("is_loaded=%s" % GDExtensionManager.is_extension_loaded(ext_path))
		# Попытка загрузить вручную, если ещё не загружено
		if not ClassDB.class_exists("DoomNode"):
			var err = GDExtensionManager.load_extension(ext_path)
			lines.append("load_ext=%s" % str(err))
			lines.append("DoomNode_after=%s" % ClassDB.class_exists("DoomNode"))
	else:
		lines.append("GDExtensionManager=NO")

	lines.append("DoomNode=%s" % ClassDB.class_exists("DoomNode"))

	if not ClassDB.class_exists("DoomNode"):
		_show("\n".join(lines))
		return
	# ========== конец диагностики ==========

	doom = ClassDB.instantiate("DoomNode")
	add_child(doom)

	# WAD вшит в libdoom
	doom.start()

	if not doom.is_running():
		_show("DoomNode есть, но start() не удался.\n" + "\n".join(lines))
		return

	_show("Doom OK " + BUILD, 3.0)


func _process(_delta: float) -> void:
	if doom and doom.is_running():
		var tex = doom.get_framebuffer_texture()
		if texture_rect.texture != tex:
			texture_rect.texture = tex


func _on_key_changed(code: int, down: bool) -> void:
	if not doom or not doom.is_running():
		return
	if down:
		doom.key_down(code)
	else:
		doom.key_up(code)


func _notification(what: int) -> void:
	if what == NOTIFICATION_APPLICATION_FOCUS_OUT and touch_ui:
		touch_ui.release_all()


func _unhandled_input(event: InputEvent) -> void:
	if not doom or not doom.is_running():
		return
	var key_event := event as InputEventKey
	if key_event and not key_event.echo:
		for code in _map_key(key_event.keycode):
			_on_key_changed(code, key_event.pressed)


func _map_key(k: Key) -> Array:
	if k >= KEY_1 and k <= KEY_7:
		return [49 + int(k) - int(KEY_1)]
	match k:
		KEY_UP, KEY_W: return [TouchUI.UP]
		KEY_DOWN, KEY_S: return [TouchUI.DOWN]
		KEY_LEFT, KEY_A: return [TouchUI.LEFT]
		KEY_RIGHT, KEY_D: return [TouchUI.RIGHT]
		KEY_SPACE, KEY_E: return TouchUI.USE
		KEY_CTRL: return TouchUI.FIRE
		KEY_SHIFT: return TouchUI.RUN
		KEY_ALT: return TouchUI.ALT
		KEY_TAB: return [TouchUI.TAB]
		KEY_ENTER, KEY_KP_ENTER: return [TouchUI.ENTER]
		KEY_ESCAPE: return [TouchUI.ESC]
	return []


# ==========================================================
#  Сенсорное управление
# ==========================================================

class TouchUI extends Control:
	signal key_changed(code: int, down: bool)

	const RIGHT := 174
	const LEFT := 172
	const UP := 173
	const DOWN := 175
	const FIRE := [157, 163]
	const USE := [32, 162]
	const ALT := [184]
	const RUN := [182]
	const ENTER := 13
	const ESC := 27
	const TAB := 9

	const MIN_HOLD_MS := 90
	const STICK_ON := 0.40
	const STICK_OFF := 0.28

	var u := 1.0
	var _buttons: Array[Dictionary] = []
	var _by_id := {}
	var _held := {}
	var _pressed_at := {}
	var _gen := {}
	var _touches := {}

	var _stick_finger := -1
	var _stick_r := 64.0
	var _stick_home := Vector2.ZERO
	var _stick_center := Vector2.ZERO
	var _stick_pos := Vector2.ZERO
	var _stick_zone := Rect2()


	func _ready() -> void:
		set_anchors_and_offsets_preset(Control.PRESET_FULL_RECT)
		mouse_filter = Control.MOUSE_FILTER_IGNORE

		_add("fire", "FIRE", FIRE, "round", Color("ff5a4a"))
		_add("use", "USE", USE, "round", Color("4adf9a"))
		_add("run", "RUN", RUN, "round", Color("ffc542"), true)
		_add("strafe", "STRAFE", ALT, "round", Color("4aa8ff"))
		_add("menu", "MENU", [ESC], "box", Color("d0d0d0"))
		_add("ok", "OK", [ENTER], "box", Color("d0d0d0"))
		_add("map", "MAP", [TAB], "box", Color("d0d0d0"))
		for i in range(1, 8):
			_add("w%d" % i, str(i), [48 + i], "box", Color("b8c4ff"))

		resized.connect(_layout)
		_layout()


	func _add(id: String, label: String, codes: Array, shape: String, color: Color, toggle := false) -> void:
		var b := {
			"label": label, "codes": codes, "shape": shape, "color": color,
			"toggle": toggle, "center": Vector2.ZERO, "r": 0.0, "rect": Rect2(),
			"down": false, "on": false,
		}
		_buttons.append(b)
		_by_id[id] = b


	func _layout() -> void:
		var s := size
		u = clampf(minf(s.x, s.y) / 540.0, 0.7, 3.0)
		var mx := 36.0 * u
		var my := 20.0 * u

		_stick_r = 64.0 * u
		_stick_home = Vector2(mx + _stick_r + 10.0 * u, s.y - my - _stick_r - 8.0 * u)
		if _stick_finger == -1:
			_stick_center = _stick_home
		_stick_zone = Rect2(0.0, s.y * 0.28, s.x * 0.42, s.y * 0.72)

		var fire := Vector2(s.x - mx - 58.0 * u, s.y - my - 58.0 * u)
		_round("fire", fire, 56.0 * u)
		_round("use", fire + Vector2(-104.0 * u, 12.0 * u), 36.0 * u)
		_round("run", fire + Vector2(-80.0 * u, -76.0 * u), 32.0 * u)
		_round("strafe", fire + Vector2(6.0 * u, -112.0 * u), 32.0 * u)

		var gap := 8.0 * u
		var x := mx
		for id in ["menu", "ok", "map"]:
			_by_id[id]["rect"] = Rect2(x, my, 56.0 * u, 30.0 * u)
			x += 56.0 * u + gap

		var cw := 32.0 * u
		var total := 7.0 * cw + 6.0 * 5.0 * u
		x = s.x - mx - total
		for i in range(1, 8):
			_by_id["w%d" % i]["rect"] = Rect2(x, my, cw, 30.0 * u)
			x += cw + 5.0 * u

		queue_redraw()


	func _round(id: String, c: Vector2, r: float) -> void:
		_by_id[id]["center"] = c
		_by_id[id]["r"] = r


	func _to_local_pos(pos: Vector2) -> Vector2:
		var inv: Transform2D = get_global_transform_with_canvas().affine_inverse()
		return inv * pos


	func _input(event: InputEvent) -> void:
		var touch := event as InputEventScreenTouch
		if touch:
			var p: Vector2 = _to_local_pos(touch.position)
			if touch.pressed:
				_touch_down(touch.index, p)
			else:
				_touch_up(touch.index)
			return

		var drag := event as InputEventScreenDrag
		if drag and drag.index == _stick_finger:
			var p: Vector2 = _to_local_pos(drag.position)
			_stick_move(p)


	func _hit(b: Dictionary, p: Vector2) -> bool:
		if b["shape"] == "round":
			return p.distance_to(b["center"]) <= float(b["r"]) * 1.15
		var rc: Rect2 = b["rect"]
		return rc.grow(6.0 * u).has_point(p)


	func _touch_down(idx: int, p: Vector2) -> void:
		for b: Dictionary in _buttons:
			if _hit(b, p):
				_touches[idx] = b
				_press_button(b)
				return
		if _stick_finger == -1 and _stick_zone.has_point(p):
			_stick_finger = idx
			_stick_center = Vector2(
				clampf(p.x, _stick_r, size.x - _stick_r),
				clampf(p.y, _stick_r, size.y - _stick_r))
			_stick_move(p)


	func _touch_up(idx: int) -> void:
		if idx == _stick_finger:
			_stick_finger = -1
			_stick_pos = Vector2.ZERO
			_stick_center = _stick_home
			_update_stick_keys(Vector2.ZERO)
			queue_redraw()
		elif _touches.has(idx):
			_release_button(_touches[idx])
			_touches.erase(idx)


	func _stick_move(p: Vector2) -> void:
		var off: Vector2 = p - _stick_center
		if off.length() > _stick_r:
			off = off.normalized() * _stick_r
		_stick_pos = off
		_update_stick_keys(off / _stick_r)
		queue_redraw()


	func _update_stick_keys(v: Vector2) -> void:
		_stick_key(UP, -v.y)
		_stick_key(DOWN, v.y)
		_stick_key(LEFT, -v.x)
		_stick_key(RIGHT, v.x)


	func _stick_key(code: int, amount: float) -> void:
		var limit: float = STICK_OFF if _held.get(code, false) else STICK_ON
		_set_key(code, amount > limit)


	func _press_button(b: Dictionary) -> void:
		b["down"] = true
		if b["toggle"]:
			b["on"] = not b["on"]
			for c in b["codes"]:
				_set_key(c, b["on"])
		else:
			for c in b["codes"]:
				_set_key(c, true)
		queue_redraw()


	func _release_button(b: Dictionary) -> void:
		b["down"] = false
		if not b["toggle"]:
			for c in b["codes"]:
				_release_later(c)
		queue_redraw()


	func _set_key(code: int, down: bool) -> void:
		if _held.get(code, false) == down:
			return
		_held[code] = down
		_gen[code] = _gen.get(code, 0) + 1
		if down:
			_pressed_at[code] = Time.get_ticks_msec()
		key_changed.emit(code, down)


	func _release_later(code: int) -> void:
		var elapsed: int = Time.get_ticks_msec() - int(_pressed_at.get(code, 0))
		var wait: int = MIN_HOLD_MS - elapsed
		if wait <= 0:
			_set_key(code, false)
			return
		var g: int = _gen.get(code, 0)
		get_tree().create_timer(wait / 1000.0).timeout.connect(func() -> void:
			if _gen.get(code, 0) == g:
				_set_key(code, false)
		)


	func release_all() -> void:
		for code in _held.keys():
			_set_key(code, false)
		for b: Dictionary in _buttons:
			b["down"] = false
			b["on"] = false
		_touches.clear()
		_stick_finger = -1
		_stick_pos = Vector2.ZERO
		_stick_center = _stick_home
		queue_redraw()


	func _draw() -> void:
		var font: Font = ThemeDB.fallback_font
		var active: bool = _stick_finger != -1

		draw_circle(_stick_center, _stick_r, Color(1, 1, 1, 0.16 if active else 0.08))
		draw_arc(_stick_center, _stick_r, 0.0, TAU, 48, Color(1, 1, 1, 0.4 if active else 0.25), 2.0 * u, true)
		draw_circle(_stick_center + _stick_pos, _stick_r * 0.42, Color(1, 1, 1, 0.55 if active else 0.28))

		for b: Dictionary in _buttons:
			var col: Color = b["color"]
			var lit: bool = b["down"] or b["on"]
			var fill := Color(col, 0.5 if lit else 0.2)
			var rim := Color(col, 0.95 if lit else 0.55)
			var txt := Color(1, 1, 1, 0.95 if lit else 0.8)

			if b["shape"] == "round":
				var r: float = float(b["r"]) * (0.93 if b["down"] else 1.0)
				draw_circle(b["center"], r, fill)
				draw_arc(b["center"], r, 0.0, TAU, 40, rim, 2.5 * u, true)
				_label(font, b["label"], b["center"], int(r * 0.42), txt)
			else:
				var rc: Rect2 = b["rect"]
				var sb := StyleBoxFlat.new()
				sb.bg_color = fill
				sb.border_color = rim
				sb.set_border_width_all(maxi(2, int(2.0 * u)))
				sb.set_corner_radius_all(int(8.0 * u))
				draw_style_box(sb, rc)
				_label(font, b["label"], rc.get_center(), int(rc.size.y * 0.5), txt)


	func _label(font: Font, text: String, center: Vector2, fs: int, color: Color) -> void:
		var w: float = font.get_string_size(text, HORIZONTAL_ALIGNMENT_LEFT, -1, fs).x
		var y: float = center.y + (font.get_ascent(fs) - font.get_descent(fs)) / 2.0
		draw_string(font, Vector2(center.x - w / 2.0, y), text, HORIZONTAL_ALIGNMENT_LEFT, -1, fs, color)