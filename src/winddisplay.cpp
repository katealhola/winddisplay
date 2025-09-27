// WindInstrument.hpp  —  LVGL 9.x, C++ wrapper (single header)
#pragma once
#include "lvgl.h"
#include <cmath>
#include <cstdio>

typedef struct {
    lv_style_t items;
    lv_style_t indicator;
    lv_style_t main;
} section_styles_t;


class WindInstrument {
public:
    // Create UI under parent (set your desired size)
    void init(lv_obj_t* parent, lv_coord_t w = 320, lv_coord_t h = 240) {
        // Root
        cont = lv_obj_create(parent);
        lv_obj_set_size(cont, w, h);
        lv_obj_center(cont);
        lv_obj_set_style_bg_color(cont, lv_color_make(28,28,28), 0);
        lv_obj_set_style_radius(cont, 10, 0);
        lv_obj_set_style_pad_all(cont, 8, 0);

        // Dial container (rotates with heading)
        dial_sz = LV_MIN(w, h) - 28;
        if(dial_sz < 160) dial_sz = 160;

        dial = lv_obj_create(cont);
        //lv_obj_remove_style_all(dial);
        lv_obj_set_size(dial, dial_sz, dial_sz);
        lv_obj_align(dial, LV_ALIGN_CENTER, 0, 8);
        //lv_obj_set_style_bg_opa(dial, LV_OPA_TRANSP, 0);
        lv_obj_set_style_transform_pivot_x(dial, dial_sz/2, 0);
        lv_obj_set_style_transform_pivot_y(dial, dial_sz/2, 0);
        lv_obj_set_style_bg_color(dial, lv_color_make(64,28,64), 0);
        

        // Circular scale
        scale = lv_scale_create(dial);
        lv_obj_set_size(scale, dial_sz - 60, dial_sz - 60);
        lv_obj_set_align(scale, LV_ALIGN_CENTER);
        lv_obj_center(scale);
        lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_INNER);
        lv_scale_set_range(scale, 0, 360);
        lv_scale_set_angle_range(scale, 360);
        lv_scale_set_rotation(scale, 180);            // 0° at top
        lv_scale_set_total_tick_count(scale, 61);     // minor every 6°
        lv_scale_set_major_tick_every(scale, 5);      // major every 30°
        lv_scale_set_label_show(scale, true);
        //lv_obj_set_style_bg_color(scale, lv_color_make(255,28,255), 0);

        lv_obj_set_style_line_color(scale, lv_color_make(180,180,180), LV_PART_ITEMS);
        lv_obj_set_style_length(scale, 8, LV_PART_ITEMS);
        lv_obj_set_style_line_width(scale, 2, LV_PART_ITEMS);
        lv_obj_set_style_line_color(scale, lv_color_white(), LV_PART_INDICATOR);
        lv_obj_set_style_length(scale, 16, LV_PART_INDICATOR);
        lv_obj_set_style_line_width(scale, 4, LV_PART_INDICATOR);
        lv_obj_set_style_text_color(scale, lv_color_white(),  LV_PART_INDICATOR);

        windscale = lv_scale_create(dial);
        lv_obj_set_size(windscale, dial_sz - 2, dial_sz - 2);
        lv_obj_set_align(windscale, LV_ALIGN_CENTER);
        lv_obj_center(windscale);
        lv_scale_set_mode(windscale, LV_SCALE_MODE_ROUND_INNER);
        lv_scale_set_range(windscale, 0, 360);
        lv_scale_set_angle_range(windscale, 360);
        lv_scale_set_rotation(windscale, -90);            // 0° at top
        lv_scale_set_total_tick_count(windscale, 61);     // minor every 6°
        lv_scale_set_major_tick_every(windscale, 5);      // major every 30°
        lv_scale_set_label_show(windscale, false);

        // Style ticks
        lv_obj_set_style_line_color(windscale, lv_color_make(255,255,0), LV_PART_ITEMS);
        lv_obj_set_style_length(windscale, 8, LV_PART_ITEMS);
        lv_obj_set_style_line_width(windscale, 2, LV_PART_ITEMS);
        lv_obj_set_style_line_color(windscale, lv_color_make(255,0,255), LV_PART_INDICATOR);
        lv_obj_set_style_length(windscale, 16, LV_PART_INDICATOR);
        lv_obj_set_style_line_width(windscale, 4, LV_PART_INDICATOR);
        lv_obj_set_style_text_color(windscale, lv_color_make(180,0,180),  LV_PART_INDICATOR);
        
        /* Zone 3: (Green) */
        init_section_styles(&greenarc_styles, lv_palette_main(LV_PALETTE_GREEN));
        add_section(windscale, 300, 360, &greenarc_styles);

        /* Zone 5: (Red) */
        init_section_styles(&redarc_styles, lv_palette_main(LV_PALETTE_RED));
        add_section(windscale, 0, 60, &redarc_styles);

        // Colored sectors (edit angles/widths to taste)
        //sector_g1 = make_sector(dial, dial_sz-4, lv_palette_main(LV_PALETTE_GREEN), 300, 360, 10);
        //sector_g2 = make_sector(dial, dial_sz-4, lv_palette_main(LV_PALETTE_GREEN),   0,  60, 10);
        //sector_r1 = make_sector(dial, dial_sz-4, lv_palette_main(LV_PALETTE_RED  ),  00,  60, 10);
        //sector_r2 = make_sector(dial, dial_sz-4, lv_palette_main(LV_PALETTE_RED  ), 300, 330, 10);

        // Cardinal letters (children of dial so they rotate with heading)
        int r = dial_sz/2 - 46;
        lbl_N = place_cardinal(dial, "N", r, -90);
        lbl_E = place_cardinal(dial, "E", r,   0);
        lbl_S = place_cardinal(dial, "S", r,  90);
        lbl_W = place_cardinal(dial, "W", r, 180);

        // Needle (separate object; we rotate it to absolute wind)
        needle = lv_line_create(cont);
        lv_obj_remove_style_all(needle);
        lv_obj_set_size(needle, dial_sz, dial_sz);
        lv_obj_align_to(needle, dial, LV_ALIGN_CENTER, 0, 0);
        needle_pts[0] = { (lv_coord_t)(dial_sz/2), (lv_coord_t)(dial_sz/2) };
        needle_pts[1] = { (lv_coord_t)(dial_sz/2), (lv_coord_t)18 };
        lv_line_set_points(needle, needle_pts, 2);
        lv_obj_set_style_line_width(needle, 6, 0);
        lv_obj_set_style_line_color(needle, lv_color_white(), 0);
        lv_obj_set_style_transform_pivot_x(needle, dial_sz/2, 0);
        lv_obj_set_style_transform_pivot_y(needle, dial_sz/2, 0);

        // Center readout
        lbl_speed_cap = lv_label_create(cont);
        lv_label_set_text(lbl_speed_cap, "Speed");
        lv_obj_align_to(lbl_speed_cap, dial, LV_ALIGN_CENTER, 0, -18);

        lbl_speed_val = lv_label_create(cont);
        lv_obj_set_style_text_font(lbl_speed_val, &lv_font_montserrat_28, 0);
        lv_label_set_text(lbl_speed_val, "0.0");
        lv_obj_align_to(lbl_speed_val, dial, LV_ALIGN_CENTER, 0, 10);

        lbl_speed_unit = lv_label_create(cont);
        lv_label_set_text(lbl_speed_unit, "Kts");
        lv_obj_align_to(lbl_speed_unit, lbl_speed_val, LV_ALIGN_OUT_RIGHT_MID, 8, 0);

        // Top-left: wind direction
        box_left = lv_obj_create(cont);
        lv_obj_set_size(box_left, 100, 48);
        lv_obj_align(box_left, LV_ALIGN_TOP_LEFT, 4, 4);
        lv_obj_set_style_radius(box_left, 6, 0);
        lv_obj_set_style_bg_color(box_left, lv_color_make(38,38,38), 0);
        lbl_left = lv_label_create(box_left);
        lv_obj_center(lbl_left);
        lv_label_set_text(lbl_left, "000°\nGWD");

        // Top-right: depth
        box_right = lv_obj_create(cont);
        lv_obj_set_size(box_right, 100, 48);
        lv_obj_align(box_right, LV_ALIGN_TOP_RIGHT, -4, 4);
        lv_obj_set_style_radius(box_right, 6, 0);
        lv_obj_set_style_bg_color(box_right, lv_color_make(38,38,38), 0);
        lbl_right = lv_label_create(box_right);
        lv_obj_center(lbl_right);
        lv_label_set_text(lbl_right, "000 ft\nDepth");

        // Bottom: heading
        lbl_heading = lv_label_create(cont);
        lv_obj_set_style_text_font(lbl_heading, &lv_font_montserrat_20, 0);
        lv_label_set_text(lbl_heading, "Hdg 000° M");
        lv_obj_align(lbl_heading, LV_ALIGN_BOTTOM_MID, 0, -4);
    }

    // Update all values: absolute wind, heading, speed, depth
    void set(float wind_dir_abs_deg,
             float heading_deg,
             float wind_speed_kts,
             float depth_ft)
    {
        // rotate dial so top reflects heading
        lv_obj_set_style_transform_angle(dial, (int16_t)(-heading_deg * 10), 0);
        // rotate needle to absolute wind
        lv_obj_set_style_transform_angle(needle, (int16_t)(wrap360(wind_dir_abs_deg) * 10), 0);

        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.1f", wind_speed_kts);
        lv_label_set_text(lbl_speed_val, buf);
        lv_label_set_text(lbl_speed_unit, "Kts");

        std::snprintf(buf, sizeof(buf), "%03d°\nGWD", (int)std::lround(wrap360(wind_dir_abs_deg)));
        lv_label_set_text(lbl_left, buf);

        std::snprintf(buf, sizeof(buf), "%d ft\nDepth", (int)std::lround(depth_ft));
        lv_label_set_text(lbl_right, buf);

        std::snprintf(buf, sizeof(buf), "Hdg %03d° M", (int)std::lround(wrap360(heading_deg)));
        lv_label_set_text(lbl_heading, buf);
    }

    // Access root if you want to move/animate externally
    lv_obj_t* root() const { return cont; }

    

private:
    // helpers
    static float wrap360(float a){ a = fmodf(a,360.f); if(a<0) a+=360.f; return a; }

    void init_section_styles(section_styles_t * styles, lv_color_t color)
    {
        lv_style_init(&styles->items);
        lv_style_set_line_color(&styles->items, color);
        lv_style_set_line_width(&styles->items, 0);

        lv_style_init(&styles->indicator);
        lv_style_set_line_color(&styles->indicator, color);
        lv_style_set_line_width(&styles->indicator, 0);

        lv_style_init(&styles->main);
        lv_style_set_arc_color(&styles->main, color);
        lv_style_set_arc_width(&styles->main, 8);
    }

    void add_section(lv_obj_t * target_scale,
                        int32_t from,
                        int32_t to,
                        const section_styles_t * styles)
    {
        lv_scale_section_t * sec = lv_scale_add_section(target_scale);
        lv_scale_set_section_range(target_scale, sec, from, to);
        lv_scale_set_section_style_items(target_scale, sec, &styles->items);
        lv_scale_set_section_style_indicator(target_scale, sec, &styles->indicator);
        lv_scale_set_section_style_main(target_scale, sec, &styles->main);
    }

    lv_obj_t* make_sector(lv_obj_t* parent, lv_coord_t sz, lv_color_t col,
                          uint16_t a1, uint16_t a2, uint16_t width)
    {
        lv_obj_t* arc = lv_arc_create(parent);
        lv_obj_set_size(arc, sz, sz);
        lv_obj_center(arc);
        lv_arc_set_bg_angles(arc, 0, 360);
        lv_obj_remove_style(arc, NULL, LV_PART_KNOB);   /*Be sure the knob is not displayed*/
        lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);  /*To not allow adjusting by click*/
        lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
        lv_obj_set_style_arc_width(arc, width, LV_PART_INDICATOR);
        lv_obj_set_style_arc_color(arc, col, LV_PART_INDICATOR);
        lv_arc_set_angles(arc, a1, a2);
        return arc;
    }

    lv_obj_t* place_cardinal(lv_obj_t* parent, const char* txt, int r, int ang_deg)
    {
        lv_obj_t* l = lv_label_create(parent);
        lv_label_set_text(l, txt);
        lv_coord_t w = lv_obj_get_width(parent), h = lv_obj_get_height(parent);
        int cx = w/2, cy = h/2;
        float a = ang_deg * (3.14159265358979323846f/180.f);
        int x = (int)(cx + r * cosf(a));
        int y = (int)(cy + r * sinf(a));
        lv_obj_set_pos(l, x - lv_obj_get_width(l)/2, y - lv_obj_get_height(l)/2);
        lv_obj_set_style_text_color(l, lv_color_white(), 0);
        return l;
    }

    // UI members
    lv_obj_t *cont=nullptr, *dial=nullptr, *scale=nullptr, *windscale=nullptr;;
    lv_obj_t *sector_g1=nullptr, *sector_g2=nullptr, *sector_r1=nullptr, *sector_r2=nullptr;
    lv_obj_t *needle=nullptr; lv_point_precise_t needle_pts[2]{};
    lv_obj_t *lbl_speed_cap=nullptr, *lbl_speed_val=nullptr, *lbl_speed_unit=nullptr;
    lv_obj_t *box_left=nullptr, *lbl_left=nullptr, *box_right=nullptr, *lbl_right=nullptr;
    lv_obj_t *lbl_heading=nullptr, *lbl_N=nullptr, *lbl_E=nullptr, *lbl_S=nullptr, *lbl_W=nullptr;
    section_styles_t greenarc_styles;
    section_styles_t redarc_styles;
    lv_coord_t dial_sz=0;
};

static WindInstrument gauge;

// Optional: demo timer hook
static void demo_timer_cb(lv_timer_t* t) {
        //auto self = static_cast<WindInstrument*>( lv_timer_get_user_data(t));
        static float a = 0.f; a += 1.f;
        //gauge.set(180.f, fmodf(a,360.f), 9.6f, 183.f);
    }

void wind_init(lv_obj_t* parent, lv_coord_t w = 320, lv_coord_t h = 240) {
    gauge.init(parent, w,h);
    // live updates
    lv_timer_create(demo_timer_cb, 5000, &gauge);

}
