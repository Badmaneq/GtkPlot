/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * gtkplot
 *
 */

#ifndef _GTK_PLOT_H_
#define _GTK_PLOT_H_

#include <gtk/gtk.h>
#include <glib-object.h>

G_BEGIN_DECLS

#define GTK_TYPE_PLOT             (gtk_plot_get_type ())
#define GTK_PLOT(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GTK_TYPE_PLOT, GtkPlot))
#define GTK_PLOT_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GTK_TYPE_PLOT, GtkPlotClass))
#define GTK_IS_PLOT(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GTK_TYPE_PLOT))
#define GTK_IS_PLOT_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GTK_TYPE_PLOT))
#define GTK_PLOT_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GTK_TYPE_PLOT, GtkPlotClass))

typedef struct _GtkPlotClass GtkPlotClass;
typedef struct _GtkPlot GtkPlot;
typedef struct _GtkPlotPrivate GtkPlotPrivate;

/* Deklaracja klasy */
struct _GtkPlotClass
{
	/* Klasa nadrzedna */
	GtkWidgetClass parent_class;

	/* Deklaracja sygnalow */
	void (* range) (GtkPlot *plot);
	void (* type) (GtkPlot *plot);
};

/* Deklaracja instacji obiektu */
struct _GtkPlot
{
	GtkWidget parent_instance;

	GtkPlotPrivate *priv;
};

typedef enum
{
	GTK_PLOT_TYPE_TOP_LEFT,
	GTK_PLOT_TYPE_BOTTOM_LEFT,
	GTK_PLOT_TYPE_CENTRE,
	GTK_PLOT_TYPE_CENTRE_LEFT,
	GTK_PLOT_TYPE_CENTRE_RIGHT,
	GTK_PLOT_TYPE_TOP_RIGHT,
	GTK_PLOT_TYPE_BOTTOM_RIGHT
	
} GtkPlotPosition;

typedef enum
{
	GTK_PLOT_LINE_TYPE_POINT        = 1 << 0,
	GTK_PLOT_LINE_TYPE_LINE         = 1 << 1,
	GTK_PLOT_LINE_TYPE_LCATMULL_ROM = 1 << 2
} GtkPlotLineType;

typedef enum
{
	GTK_PLOT_GIRD_X = 1 << 0,
	GTK_PLOT_GIRD_Y = 1 << 1
} GtkPlotGird;

/* Funkcja zwraca identyfikator klasy. Na potrzeby nowo tworzonego obiektu. */
GType gtk_plot_get_type (void) G_GNUC_CONST;
/* Funkcja tworzy nowy obiekt */
GtkWidget* gtk_plot_new(void);

/* Publiczne "metody" operujace na obiekcie */
void gtk_plot_add_data(GtkPlot *plot, GArray *array);
void gtk_plot_set_line_type(GtkPlot *plot, GtkPlotLineType plot_line_type);
void gtk_plot_set_type(GtkPlot *plot, GtkCornerType plot_type);
void gtk_plot_set_reference_range(GtkPlot *plot, guint lower, guint upper);
void gtk_plot_set_line_width(GtkPlot *plot, gdouble width);
void gtk_plot_set_spline_factor(GtkPlot *plot, gdouble width);
void gtk_plot_set_color(GtkPlot *plot, GtkPlotLineType plot_line_type, GdkColor color);
void gtk_plot_set_gird(GtkPlot *plot, GtkPlotGird gird);

G_END_DECLS

#endif /* _GTK_PLOT_H_ */

