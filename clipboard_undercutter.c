#include <stdio.h>
#include <string.h>

#include <gtk/gtk.h>
#include <glib.h>

static gint64 exp10(int exponent) {
  int i;
  gint64 n = 1;

  for (i = 0; i < exponent; ++i)
    n *= 10;

  return n;
}

static gint64 extract_isk_amount(const gchar* text) {
  const gchar* space_isk;
  const gchar* p;
  gint64 num;
  int digits;

  if (!text)
    return -1;

  space_isk = strstr(text, " ISK\t");
  if (!space_isk)
    return -1;

  num = 0;
  digits = 0;
  for (p = space_isk - 1; p > text; --p) {
    char c = *p;
    switch (c) {
      case '\t':
        p = text;
        break;
      case '0': case '1': case '2': case '3': case '4':
      case '5': case '6': case '7': case '8': case '9':
        num = num + (gint64) (c - '0') * exp10(digits++);
        break;
      case '.':
        if (p != space_isk - 3)
          return -1;
        break;
      case ',':
        if ((space_isk - p - 3) % 4 != 0)
          return -1;
        break;
      default:
        return -1;
    }
  }

  return num;
}

static void on_text(
      GtkClipboard* clipboard, const gchar* text, gpointer user_data) {
  gint64 isk;
  char buf[32];
  gint len;
  gint64 price_offset = *(gint64*) user_data;

  isk = extract_isk_amount(text);
  if (isk == -1)
    return;

  isk += price_offset;
  len = g_snprintf(
      buf, sizeof buf,
      "%" G_GINT64_FORMAT ".%02" G_GINT64_FORMAT,
      isk / 100, isk % 100);

  gtk_clipboard_set_text(clipboard, buf, len);
}

static void on_owner_change(
      GtkClipboard* clipboard, GdkEvent* generic_event, gpointer user_data) {
  gtk_clipboard_request_text(clipboard, on_text, user_data);
}

struct OffsetOption {
  const char* label;
  gint64 my_offset;
  gint64* offset;
};

static void on_radio_toggle(GtkToggleButton* button, gpointer user_data) {
  struct OffsetOption* option = (struct OffsetOption*) user_data;

  if (gtk_toggle_button_get_active(button))
    *option->offset = option->my_offset;
}

static void on_destroy(GtkWidget* widget, gpointer user_data) {
  gtk_main_quit();
}

static void make_gui(gint64* price_offset) {
  GtkWidget* window;
  GtkWidget* box;
  GtkWidget* label;
  GtkWidget* radio;
  static struct OffsetOption options[] = {
    { "For Sell orders (-0.01 ISK)", -1, NULL },
    { "No adjustment (+/- 0.00 ISK)", 0, NULL },
    { "For Buy orders (+0.01 ISK)", 1, NULL }
  };
  struct OffsetOption* p_option;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 10);
  gtk_window_set_title(GTK_WINDOW(window), "Clipboard Undercutter");
  gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
  g_signal_connect(window, "destroy", G_CALLBACK(on_destroy), NULL);

  box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

  label = gtk_label_new("Price adjustment:");
  gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(label), FALSE, FALSE, 4);

  radio = NULL;
  for (p_option = options;
       p_option < options + sizeof(options)/sizeof(*options);
       ++p_option) {
    radio = gtk_radio_button_new_with_label_from_widget(
        GTK_RADIO_BUTTON(radio), p_option->label);
    gtk_box_pack_start(GTK_BOX(box), GTK_WIDGET(radio), FALSE, FALSE, 0);
    g_signal_connect(
        radio, "toggled",
        G_CALLBACK(on_radio_toggle), (gpointer) p_option);
    p_option->offset = price_offset;
  }

  gtk_container_add(GTK_CONTAINER(window), GTK_WIDGET(box));
  gtk_widget_show_all(GTK_WIDGET(window));
}

int main(int argc, char* argv[]) {
  GtkClipboard* clipboard;
  gint64 price_offset;

  gtk_init(&argc, &argv);

  clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);

  price_offset = -1;

  g_signal_connect(
      clipboard, "owner-change",
      G_CALLBACK(on_owner_change), (gpointer) &price_offset);

  make_gui(&price_offset);

  gtk_main();

  return 0;
}
