/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * gtkplot
 *
 */


#include "gtk-plot.h"

/*zmienic na prywatne*/
gdouble min_x=G_MAXDOUBLE, max_x=G_MINDOUBLE, min_y=G_MAXDOUBLE, max_y=G_MINDOUBLE;
#define MIN_X 20
#define MIN_Y 20

struct punkt
{
	gdouble x;
	gdouble y;
};


/* Strukura przechowujaca prywatne zmienne obiektu */
struct _GtkPlotPrivate
{
	GArray     *points; // Typ danych
	GtkPlotLineType visible_type; // Jakiego rodzaju linie maja byc widoczne
/*	GtkPlotType*/
	GtkCornerType chart_type; // Polozenie
	gdouble     spline_factor; // Wspolczynnik wypelnienia
	GtkWidget  *draw; //Kontrolka obszaru rysunkowego
	guint      lower; // Wysokosc dolnego prostokata referycyjnego
	guint      upper; // Wysokosc gornego prostokata referycyjnego
	gdouble    line_width; // szerokosc linii
	GdkColor   color[3];
	GtkPlotGird gird;
};

/* Wlasciwosci jakie posiada kontrolka */
enum
{
	PROP_0,

	PROP_REFERENCE_RANGE_LOWER,
	PROP_REFERENCE_RANGE_UPPER,
	PROP_TYPE,
	PROP_DATA_ADD
};

/* Sygnaly */
enum
{
	RANGE,
	TYPE,

	LAST_SIGNAL
};

static guint plot_signals[LAST_SIGNAL] = { 0 };

/* Makro pobierajace pywatne zmienne obiektu */
#define GTK_PLOT_GET_PRIVATE(o)  (G_TYPE_INSTANCE_GET_PRIVATE ((o), GTK_TYPE_PLOT, GtkPlotPrivate))

/* Prototypy funkcji statycznych zwiazanych z sygnalami okna i kontrolki */
static void gtk_plot_realize(GtkWidget *widget);
static gboolean gtk_plot_expose(GtkWidget *widget, GdkEventExpose *event);
static gboolean gtk_plot_button_press_event (GtkWidget *widget, GdkEventButton *event);
static void gtk_plot_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
//static void gtk_plot_destroy(GtkObject *object);

/* Prototypy funkcji statycznych zwiazanych z rysowaniem wykresu */
static void draw_points( cairo_t *cr, gdouble *coords, gint size, GdkColor color);
static void draw_line( cairo_t *cr, gdouble *coords, gint size, gint width, gint height, GtkCornerType plot_type, GdkColor color);
static void draw_catmull_rom( cairo_t *cr, gdouble *coords, gint size, double spline_factor);
static void paint_referce_reference_range(GtkPlot *plot, cairo_t *cr);
static void draw_gird_lines(GtkPlot *plot, cairo_t *cr);

static void find_min_max(GArray *array);

/* Makro, które tworzy funkcje gtk_plot_get_type, pozwala zaoszczędzić na ilośći kodu.
   Pierwszy parametr rejestruje obiekt GtkPlot, drugi decyduje jak bedą sie rozpoczynać 
   funkcje init i class_init, parametr GTK_TYPE_WIDGET świadzczy, że dziedziczy po obiekcie
   GtkWidget */
G_DEFINE_TYPE (GtkPlot, gtk_plot, GTK_TYPE_WIDGET);

/* Tworzy nowy obiekt kontrolki */
GtkWidget* gtk_plot_new(void)
{
   return GTK_WIDGET( gtk_object_new(GTK_TYPE_PLOT, NULL) );
}

/* Ala konstruktor instacji obiektu */
static void gtk_plot_init (GtkPlot *plot)
{
	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot); 

	/* Dodanie kontrolce sygnalu */
//	gtk_widget_add_events( GTK_WIDGET(plot), GDK_BUTTON_PRESS_MASK | GDK_CONFIGURE );

	/* Ustawienie domyslnych parametrow kontrolki */
	priv->draw = gtk_drawing_area_new ();
	priv->points = NULL;
	priv->visible_type = GTK_PLOT_LINE_TYPE_POINT;
	priv->chart_type = GTK_CORNER_TOP_LEFT;
/*		GTK_PLOT_TYPE_TOP | GTK_PLOT_TYPE_LEFT;*/
	priv->lower = 0;
	priv->upper = 0;
	priv->line_width = 1.0;

	priv->color[0].red = 65535;
	priv->color[0].green = 0;
	priv->color[0].blue = 0;

	priv->color[1].red = 0;
	priv->color[1].green = 0;
	priv->color[1].blue = 65535;

	priv->color[2].red = 0;
	priv->color[2].green = 65535;
	priv->color[2].blue = 0;

	priv->gird = 0;
}

static void
gtk_plot_finalize (GObject *object)
{
		/* TODO: Add deinitalization code here */

		G_OBJECT_CLASS (gtk_plot_parent_class)->finalize (object);
}

static void gtk_plot_range (GtkPlot *self)
{
	/* TODO: Add default signal handler implementation here */
	//paint_referce_reference_range (self
	g_print ( "sygnal !!!\n" );
}

/* Obsluga ustawiania opcji/ wlasciwosci */
static void gtk_plot_set_property (GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
	GtkPlot *plot = GTK_PLOT(object);
	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot); 
	
	switch (prop_id)
	{
		case PROP_REFERENCE_RANGE_LOWER:
			priv->lower = g_value_get_uint ( value );
			break;
			
		case PROP_REFERENCE_RANGE_UPPER:
			priv->upper = g_value_get_uint ( value );
			break;
			
		case PROP_TYPE:
			priv->visible_type = g_value_get_uchar ( value );
			break;

		case PROP_DATA_ADD:
/*			priv->points = (GArray *)g_value_get_pointer ( value );*/
			gtk_plot_add_data ( plot, (GArray *)g_value_get_pointer ( value ) );
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}
/* Obsluga pobierania opcji/wlasciwosci */
static void gtk_plot_get_property (GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
	GtkPlot *plot = GTK_PLOT(object);
	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot); 
	g_return_if_fail (GTK_IS_PLOT (object));

	switch (prop_id)
	{
		case PROP_REFERENCE_RANGE_LOWER:
			g_value_set_uint ( value, priv->lower );
			break;

		case PROP_REFERENCE_RANGE_UPPER:
			g_value_set_uint ( value, priv->upper );
			break;
			
		case PROP_TYPE:
			g_value_set_uint ( value, priv->upper );
			break;

		default:
			G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
			break;
	}
}

/* Inicjalizacja klasy */
static void gtk_plot_class_init (GtkPlotClass *klass)
{
	GObjectClass* object_class = G_OBJECT_CLASS (klass);
	GtkWidgetClass* widget_class = GTK_WIDGET_CLASS (klass);

	GtkWidgetClass *parent_class = gtk_type_class (gtk_widget_get_type ());

	/* Dodaje klasie prywatne zmienne */
	g_type_class_add_private (klass, sizeof (GtkPlotPrivate));

	widget_class->realize       = gtk_plot_realize;
	widget_class->expose_event  = gtk_plot_expose;
	widget_class->button_press_event = gtk_plot_button_press_event;
	widget_class->size_allocate = gtk_plot_size_allocate;

	/* Przeciazanie metod klasy G_OBJECT_CLASS */
	//object_class->finalize = gtk_plot_finalize;
	object_class->set_property = gtk_plot_set_property;
	object_class->get_property = gtk_plot_get_property;
	
	klass->range = gtk_plot_range; 

	/* Instalacja wlasciwosci kontrolki */
	g_object_class_install_property (object_class,
	                                 PROP_REFERENCE_RANGE_LOWER,
	                                 g_param_spec_uint ("range-lower",
	                                                    "range",
	                                                    "Ustawia / pobiera range-lower",
	                                                    0,
	                                                    G_MAXUINT,
	                                                    0,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE));

	g_object_class_install_property (object_class,
	                                 PROP_REFERENCE_RANGE_UPPER,
	                                 g_param_spec_uint ("range-upper",
	                                                    "range",
	                                                    "Ustawia / pobiera range-upper",
	                                                    0,
	                                                    G_MAXUINT,
	                                                    0,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE));

	g_object_class_install_property (object_class,
	                                 PROP_TYPE,
	                                 g_param_spec_uchar ("type",
	                                                    "type",
	                                                    "Ustawia / pobiera typ lini wykresu",
	                                                     0,
	                                                    GTK_PLOT_LINE_TYPE_POINT |
	                                                    GTK_PLOT_LINE_TYPE_LINE |
	                                                    GTK_PLOT_LINE_TYPE_LCATMULL_ROM,
	                                                    GTK_PLOT_LINE_TYPE_POINT,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE));
	/*g_object_class_install_property (object_class,
	                                 PROP_TYPE,
	                                 g_param_spec_enum ("type",
	                                                    "type",
	                                                    "Ustawia / pobiera typ lini wykresu",
	                                                    GtkPlotType,
	                                                    GTK_PLOT_TYPE_POINT,
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE));*/

	
	g_object_class_install_property (object_class,
	                                 PROP_DATA_ADD,
	                                 g_param_spec_pointer ("data",
	                                                    "data",
	                                                    "Ustawia / pobiera dane wykresu",
	                                                    G_PARAM_READABLE | G_PARAM_WRITABLE));
	/* Rejestracja sygnalow */
  plot_signals[RANGE] = g_signal_new ("range",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_LAST,
                                      G_STRUCT_OFFSET (GtkPlotClass, range),
                                      NULL /* accumulator */,
                                      NULL /* accu_data */,
                                      g_cclosure_marshal_VOID__VOID,
                                      G_TYPE_NONE /* return_type */,
                                      0 /* n_params */ );

	plot_signals[TYPE] = g_signal_new ("type",
                                      G_TYPE_FROM_CLASS (klass),
                                      G_SIGNAL_RUN_LAST,
                                      G_STRUCT_OFFSET (GtkPlotClass, type),
                                      NULL /* accumulator */,
                                      NULL /* accu_data */,
                                      g_cclosure_marshal_VOID__VOID,
                                      G_TYPE_NONE /* return_type */,
                                      0 /* n_params */ );
}

/* Funkcja wiazaca kontrolke z oknem */
static void gtk_plot_realize(GtkWidget *widget)
{
  GdkWindowAttr attributes;
  guint attributes_mask;
  GtkAllocation allocation;

  g_return_if_fail(widget != NULL);
  g_return_if_fail(GTK_IS_PLOT(widget));

/*  GTK_WIDGET_SET_FLAGS(widget, GTK_REALIZED);*/
	gtk_widget_set_realized ( widget, TRUE );

  gtk_widget_get_allocation (widget, &allocation);

  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = widget->allocation.x;
  attributes.y = widget->allocation.y;
  attributes.width = allocation.width; 
  attributes.height = allocation.height;

  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.event_mask = gtk_widget_get_events(widget) | GDK_EXPOSURE_MASK |GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK;

  attributes_mask = GDK_WA_X | GDK_WA_Y;

  widget->window = gdk_window_new ( gtk_widget_get_parent_window (widget),
                                   &attributes, attributes_mask );

  gdk_window_set_user_data(widget->window, widget);

  widget->style = gtk_style_attach(widget->style, widget->window);
  gtk_style_set_background(widget->style, widget->window, GTK_STATE_NORMAL);
}

/* Funkcja reagujaca na zmiane/odsloniecie kontolki */
static gboolean gtk_plot_expose(GtkWidget *widget, GdkEventExpose *event)
{
	cairo_t  *cr;
	gdouble  *coords;

	g_return_val_if_fail(widget != NULL, FALSE);
	g_return_val_if_fail(GTK_IS_PLOT(widget), FALSE);
	g_return_val_if_fail(event != NULL, FALSE);


	GtkPlot *gtk_plot = GTK_PLOT(widget);
	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(gtk_plot);


	if( priv->points == NULL )
		return ( FALSE );

	if( priv->points->len == 0 )
		return ( FALSE );

	/* Sprawdza czy kontrokla moze byc narysowana */
	if ( gtk_widget_is_drawable (widget) )
	{
		cr = gdk_cairo_create( widget->window );
		
		coords = (gdouble *)priv->points->data;

		/* Rysuj linie odniesienia */
		draw_gird_lines ( gtk_plot, cr );

		cairo_set_line_width( cr, priv->line_width );
		
		/* Rysuje 1 punkt */
		if( priv->points->len == 2 && priv->visible_type & GTK_PLOT_LINE_TYPE_POINT )
		{
			draw_points( cr, coords, priv->points->len, priv->color[0] );
      cairo_destroy( cr );
      return( FALSE );
		}

		/* Rysuje linie */
		if( priv->visible_type & GTK_PLOT_LINE_TYPE_LINE  )
		{
			GtkAllocation allocation;
			gint width, height;
			
			gtk_widget_get_allocation ( widget, &allocation );
			width = allocation.width;
	  	height = allocation.height;

      draw_line( cr, coords, priv->points->len, width, height, priv->chart_type, priv->color[1] );
	  }

		/* Rysuje zaokraglenia */
    if( priv->visible_type & GTK_PLOT_LINE_TYPE_LCATMULL_ROM  )
      draw_catmull_rom( cr, coords, priv->points->len, priv->spline_factor );

		/* Rysuje punkty */
    if( priv->visible_type & GTK_PLOT_LINE_TYPE_POINT )
      draw_points( cr, coords, priv->points->len, priv->color[0] );

		/* Rysuje referencyjne prostokaty */
		paint_referce_reference_range ( gtk_plot, cr );

		cairo_destroy( cr );
	}

  return FALSE;

}

static void gtk_plot_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
/*	GtkPlot *plot;*/
	
	g_return_if_fail (widget != NULL);
  g_return_if_fail (GTK_IS_PLOT (widget));
  g_return_if_fail (allocation != NULL);
	
/*	plot = GTK_PLOT(widget);*/
	widget->allocation = *allocation;

	if ( gtk_widget_get_realized(widget) )
    gdk_window_move_resize (widget->window,
                            allocation->x, 
                            allocation->y,
                            allocation->width, 
                            allocation->height);

}

static gboolean gtk_plot_button_press_event (GtkWidget *widget, GdkEventButton *event)
{

		//g_print ("x=%d\n", event->x);
	g_print ("<<<<<<<<<DUPA>>>>>>>>\n");
		return FALSE;
}

static struct punkt recalculate_points(gdouble point[2], gint width, gint height)
{
	struct punkt przeliczony_punkt;
	
	/* Przeliczanie X */
	g_print ("X=%f   Y=%f\n", point[0], point[1]);
	przeliczony_punkt.x = (point[0] - min_x) * (width   / (max_x - min_x));
	/*Przeliczanie Y */
	przeliczony_punkt.y = height - (point[1] - min_y) * (height / (max_y-min_y));

	return przeliczony_punkt;
}

/** Funkcje rysujace **/
/* Rysuje punkty */
static void draw_points( cairo_t *cr, gdouble *coords, gint size, GdkColor color )
{
	gint i;

	/* Ustawia kolor punktu */
	cairo_set_source_rgb( cr, color.red, color.green, color.blue );

	/* Ustawia rozmiar punktu */
	cairo_set_line_width( cr, 5.0 );

	for( i = 0; i < size; i += 2 )
	{
		cairo_move_to( cr, coords[i], coords[i + 1] );
		cairo_arc( cr, coords[i], coords[i + 1], 2, 0, 2 * G_PI );
	}

	cairo_stroke( cr );
}

/* Rysuje linie */
static void draw_line( cairo_t *cr, gdouble *coords, gint size, gint width, gint height, GtkCornerType plot_type,  GdkColor color)
{
	gint i;
	struct punkt point;

	/* Ustawia kolor */
	cairo_set_source_rgb( cr, color.red, color.green, color.blue );

	if ( plot_type == GTK_CORNER_TOP_LEFT )
	{
		cairo_move_to( cr, coords[0], coords[1] );

		for( i = 2; i < size; i += 2 )
      cairo_line_to( cr, coords[i], coords[i + 1] );
	}

	if ( plot_type == GTK_CORNER_BOTTOM_LEFT )
	{
/*	 point = recalculate_points (coords, width, height);*/
   cairo_move_to( cr, coords[0], height - coords[1] );
/*	 cairo_move_to( cr, point.x, point.y );*/
   for( i = 2; i < size; i += 2 )
   {
		  //point = recalculate_points (&coords[i], width,height);
		  /*cairo_line_to( cr, point.x, point.y );*/
      cairo_line_to( cr, coords[i], height - coords[i + 1] );
   }
	}

	if ( plot_type == GTK_CORNER_TOP_RIGHT )
	{
		cairo_move_to( cr, width - coords[0], coords[1] );

		for( i = 2; i < size; i += 2 )
      cairo_line_to( cr, width - coords[i], coords[i + 1] );
	}

	if ( plot_type == GTK_CORNER_BOTTOM_RIGHT )
	{
		cairo_move_to( cr, width - coords[0], coords[1] );

		for( i = 2; i < size; i += 2 )
      cairo_line_to( cr, width - coords[i], height - coords[i + 1] );
	}
	
	cairo_stroke( cr );
}

static void
draw_catmull_rom( cairo_t *cr,
              gdouble *coords,
              gint     size,
              double   spline_factor )
{
   gint i;
   gdouble ctrl_points[4];

   /* Initial calculations */
   ctrl_points[2] = ( coords[2] - coords[0] ) / spline_factor;
   ctrl_points[3] = ( coords[3] - coords[1] ) / spline_factor;

   cairo_set_source_rgb( cr, 0, 1, 0 );
   cairo_move_to( cr, coords[0], coords[1] );
   for( i = 2; i < size - 2; i += 2 )
   {
      ctrl_points[0] = ctrl_points[2];
      ctrl_points[1] = ctrl_points[3];
      ctrl_points[2] = ( coords[i + 2] - coords[i - 2] ) / spline_factor;
      ctrl_points[3] = ( coords[i + 3] - coords[i - 1] ) / spline_factor;

      cairo_curve_to( cr, coords[i - 2] + ctrl_points[0],
                     coords[i - 1] + ctrl_points[1],
                     coords[i]     - ctrl_points[2],
                     coords[i + 1] - ctrl_points[3],
                     coords[i],
                     coords[i + 1] );
   }

   /* Draw the last segment */
   ctrl_points[0] = ctrl_points[2];
   ctrl_points[1] = ctrl_points[3];
   ctrl_points[2] = ( coords[i] - coords[i - 2] ) / spline_factor;
   ctrl_points[3] = ( coords[i + 1] - coords[i - 1] ) / spline_factor;

   cairo_curve_to( cr, coords[i - 2] + ctrl_points[0],
                  coords[i - 1] + ctrl_points[1],
                  coords[i]     - ctrl_points[2],
                  coords[i + 1] - ctrl_points[3],
                  coords[i],
                  coords[i + 1] );

   cairo_stroke( cr );
}

/* Rysuje prostokaty referencyjne */
static void paint_referce_reference_range(GtkPlot *plot, cairo_t *cr)
{
		
		GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot);

		GtkWidget *widget;
		GtkAllocation allocation;
		gint x, y;
		gint width, height;

		widget = GTK_WIDGET(priv->draw);
		x = widget->allocation.x;
		y = widget->allocation.y;

		gtk_widget_get_allocation ( GTK_WIDGET(plot), &allocation );
		width = allocation.width;
		height = allocation.height;

		/* Ustawia kolor wypełnienia */
		cairo_set_source_rgb ( cr, 1.0, 0.0, 0.0 );
		/* Rysuje gorne wypelnienie */
		cairo_rectangle ( cr, x, y, width, priv->lower );
		/* Rysuje dolne wypelnienie */
		cairo_rectangle ( cr, x, height - priv->upper, width, priv->upper );
		cairo_clip ( cr );
		cairo_paint_with_alpha ( cr, 0.5 );
		
}

/* Rysuje linie odniesienia */
static void draw_gird_lines(GtkPlot *plot, cairo_t *cr)
{
	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot);
	GtkWidget *widget;
	GtkAllocation allocation;
	gint x, y;
	gint width, height;
	guint i;

	widget = GTK_WIDGET(priv->draw);


	gtk_widget_get_allocation ( GTK_WIDGET(plot), &allocation );
	width = allocation.width;
	height = allocation.height;
	x = allocation.x;
	y = allocation.y;
//	x = widget->allocation.x;
	y = widget->allocation.y;

	
	if ( priv->gird & GTK_PLOT_GIRD_Y )
	{
		cairo_set_source_rgb( cr, 0.8, 0.8, 0.8 ); // Ustawia kolor "szary
  /* Rysowanie linii pionowych */
  for ( i = x + MIN_X; i < width; i += MIN_X )
  {

    cairo_move_to ( cr, i, y); // Ustawia pióro na wsp. x,y
    cairo_line_to ( cr, i, height ); // Rysuje linie
  }
  cairo_stroke ( cr ); // Wyswietla na kontrolce
	}

	if ( priv->gird & GTK_PLOT_GIRD_X )
	{
		cairo_set_source_rgb( cr, 0.8, 0.8, 0.8 ); // Ustawia kolor "szary
  /* Rysowanie linii poziomych */
  for ( i = y + MIN_Y; i < height; i += MIN_Y )
  {
    cairo_move_to ( cr, x, i);
    cairo_line_to ( cr, width, i );
  }
  cairo_stroke ( cr ); // Wyswietla na kontrolce
	}
	
}


/**********************************************/
/** Publiczne "metody" operujace na obiekcie **/
/**********************************************/

/* Dodaje dane do kontrokli */
void gtk_plot_add_data(GtkPlot *plot, GArray *array)
{
	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot);

	priv->points = array;

	find_min_max ( array );
}


/* Ustawia typ linii wykresu */
void gtk_plot_set_line_type(GtkPlot *plot, GtkPlotLineType plot_type)
{
	g_return_if_fail ( GTK_IS_PLOT(plot) );

	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot);

    if ( plot_type & GTK_PLOT_LINE_TYPE_POINT )
    {
      priv->visible_type |= GTK_PLOT_LINE_TYPE_POINT;

    }
    else
    {
        priv->visible_type &= ~GTK_PLOT_LINE_TYPE_POINT;
    }


    if ( plot_type & GTK_PLOT_LINE_TYPE_LINE )
    {

        priv->visible_type |= GTK_PLOT_LINE_TYPE_LINE;
        
    }
    else
    {
        priv->visible_type &= ~GTK_PLOT_LINE_TYPE_LINE;
    }

    if ( plot_type & GTK_PLOT_LINE_TYPE_LCATMULL_ROM )
    {

         priv->visible_type |= GTK_PLOT_LINE_TYPE_LCATMULL_ROM;

    }
    else
    {
        priv->visible_type &= ~GTK_PLOT_LINE_TYPE_LCATMULL_ROM;
    }

    gtk_widget_queue_draw (GTK_WIDGET (plot));

	g_signal_emit ( plot, plot_signals[TYPE], 0 );
}

/* Ustawia typ wykresu */
void gtk_plot_set_type(GtkPlot *plot, GtkCornerType plot_type)
{
	g_return_if_fail ( GTK_IS_PLOT(plot) );
	
	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot);

	priv->chart_type = plot_type;
	gtk_widget_queue_draw (GTK_WIDGET (plot));
}

/* Ustawia rozmiar referencyjnych prostokatow */
void gtk_plot_set_reference_range(GtkPlot *plot, guint lower, guint upper)
{
	g_return_if_fail ( GTK_IS_PLOT(plot) );
	
	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot);

	priv->lower = lower;
	priv->upper = upper;
	
	gtk_widget_queue_draw (GTK_WIDGET (plot));
	g_signal_emit ( plot, plot_signals[RANGE], 0 );
}

/* Ustawia szrokosc linii */
void gtk_plot_set_line_width(GtkPlot *plot, gdouble width)
{
		GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot);

		priv->line_width = width;
		gtk_widget_queue_draw (GTK_WIDGET (plot));
}

/* Ustawia wspolczyniki zaokraglenia */
void gtk_plot_set_spline_factor(GtkPlot *plot, gdouble factor)
{
	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot);

	priv->spline_factor = factor;
	gtk_widget_queue_draw (GTK_WIDGET (plot));
}

/* Ustawia kolor wybranego typu linii */
void gtk_plot_set_color(GtkPlot *plot, GtkPlotLineType plot_line_type, GdkColor color)
{
	g_return_if_fail ( GTK_IS_PLOT(plot) );

	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot);

	if ( plot_line_type & GTK_PLOT_LINE_TYPE_POINT )
		priv->color[0] = color;

	if ( plot_line_type & GTK_PLOT_LINE_TYPE_LINE )
		priv->color[1] = color;

	if ( plot_line_type & GTK_PLOT_LINE_TYPE_LCATMULL_ROM )
		priv->color[2] = color;

	gtk_widget_queue_draw (GTK_WIDGET (plot));
}

/* Ustawia czy rysowac linie odniesienia */
void gtk_plot_set_gird(GtkPlot *plot, GtkPlotGird gird)
{
	GtkPlotPrivate *priv = GTK_PLOT_GET_PRIVATE(plot);
	
	if ( gird & GTK_PLOT_GIRD_X )
		priv->gird |= GTK_PLOT_GIRD_X;
	else
		priv->gird &= ~GTK_PLOT_GIRD_X;

	if ( gird & GTK_PLOT_GIRD_Y )
		priv->gird |= GTK_PLOT_GIRD_Y;
	else
		priv->gird &= ~GTK_PLOT_GIRD_Y;

	gtk_widget_queue_draw (GTK_WIDGET (plot));
}

static void find_min_max (GArray *array)
{
	int i;
	gdouble *coords;

	coords = (gdouble *)array->data;
	for ( i = 0; i < array->len; i += 2 )
	{
	if ( coords[i] < min_x ) 
		min_x = coords[i];
	
  if ( coords[i] > max_x ) 
		max_x = coords[i];

	if ( coords[i+1] < min_y )
		min_y = coords[i+1];

	if ( coords[i+1] > max_y ) 
		max_y = coords[i+1];
	}

	 g_print ( "min_x=%f, max_x=%f, min_y=%f, max_y=%f\n", min_x, max_x, min_y, max_y );
}

