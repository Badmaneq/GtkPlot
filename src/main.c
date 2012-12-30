/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*-  */
/*
 * main.c
 * Copyright (C) 2012 badman <badman@localhost.localdomain>
 *
 */


#include <gtk/gtk.h>
#include "gtk-plot.h"

/* Nalezy pzomieniac wszytsko
 * co sie da */

enum
{
   SPLINE_POINT = 0,
   SPLINE_LINE,
   SPLINE_CATMULL_ROM,
   NO_TYPES
};

static GtkWidget*
create_window (void)
{
		GtkWidget *window;

		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title (GTK_WINDOW (window), "gtkplot");
		gtk_window_set_default_size ( GTK_WINDOW(window), 300, 200 );

		g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);

		return window;
}

void cb_toggled( GtkToggleButton *button, gpointer data )
{
    GtkPlot *plot;
    static GtkPlotLineType typ = GTK_PLOT_LINE_TYPE_LINE;
    //static guchar typ = SPLINE_LINE;
    gint index = GPOINTER_TO_INT( g_object_get_data( G_OBJECT( button ), "type" ) );

    plot = (GtkPlot *) data;

    if ( index == SPLINE_POINT )
    {
        if (  gtk_toggle_button_get_active( button ) )
            typ |= GTK_PLOT_LINE_TYPE_POINT;
        else
            typ &= ~GTK_PLOT_LINE_TYPE_POINT;
    }

    if ( index == SPLINE_LINE )
    {
			gtk_plot_set_type ( plot, GTK_POS_BOTTOM );
       if (  gtk_toggle_button_get_active( button ) )
            typ |= GTK_PLOT_LINE_TYPE_LINE;
        else
            typ &= ~GTK_PLOT_LINE_TYPE_LINE;
    }

    if ( index == SPLINE_CATMULL_ROM )
    {
       if (  gtk_toggle_button_get_active( button ) )
            typ |= GTK_PLOT_LINE_TYPE_LCATMULL_ROM;
        else
            typ &= ~GTK_PLOT_LINE_TYPE_LCATMULL_ROM;
    }


    g_print ( "Typ=%d\n", typ );
    gtk_plot_set_line_type ( plot, typ );
		gtk_plot_set_reference_range ( plot, 50, 30 );

}

void cb_value_line( GtkSpinButton *button, gpointer data )
{
		gdouble wartosc;

		wartosc = gtk_spin_button_get_value( button );
		gtk_plot_set_line_width ( GTK_PLOT(data), wartosc );
}

void cb_value_spline( GtkSpinButton *button, gpointer data )
{
		gdouble wartosc;

		wartosc = gtk_spin_button_get_value( button );
		gtk_plot_set_spline_factor ( GTK_PLOT(data), wartosc );
		
}

gboolean cb_press( GtkWidget *draw, GdkEventButton *event, gpointer data )
{
   if( event->state & GDK_SHIFT_MASK )
   {
     // gint     i;
      /*gdouble *coords = (gdouble *)data->points->data;

      for( i = 0; i < data->points->len - 1; i += 2 )
      {
         if( event->x > ( coords[i] - 5 ) &&
            event->x < ( coords[i] + 5 ) )
         {
            if( event->y > ( coords[i + 1] - 5 ) &&
               event->y < ( coords[i + 1] + 5 ) )
            {
               g_print( "Delete point at (%f, %f)\n", event->x, event->y );
             //  g_array_remove_range( data->points, i, 2 );
               break;
            }
         }
      }*/
   }
   else
   {
      gdouble point[] = { event->x, event->y };

      g_print( "Create point at (%f, %f)\n", event->x, event->y );
      g_array_append_vals( (GArray *)data, (gconstpointer)point, 2 );
		  gtk_plot_set_reference_range ( GTK_PLOT(draw), event->y, 20 );
   }


   return( TRUE );
}


void cb_combo(GtkComboBox *combo, gpointer user_data)
{
	gint pos;
	GtkPlot *plot = (GtkPlot *) user_data;
	
	pos = gtk_combo_box_get_active ( combo );

	switch ( pos )
	{
		case 0:
			gtk_plot_set_type ( plot, GTK_CORNER_TOP_LEFT );
			break;

		case 1:
			gtk_plot_set_type ( plot, GTK_CORNER_BOTTOM_LEFT );
			break;

		case 2:
			gtk_plot_set_type ( plot, GTK_CORNER_TOP_RIGHT );
			break;

		case 3:
			gtk_plot_set_type ( plot, GTK_CORNER_BOTTOM_RIGHT );
			break;

	}
	
}

void  cb_color(GtkColorButton *button, gpointer user_data)
{
	GdkColor color;
	GtkPlot *plot = (GtkPlot *) user_data;
	
	gtk_color_button_get_color ( button, &color );
	gtk_plot_set_color ( plot, GTK_PLOT_LINE_TYPE_LINE, color );
}

void cb_gird(GtkToggleButton *button, gpointer user_data)
{
	static GtkPlotGird gird = GTK_PLOT_GIRD_X;
	gint index = GPOINTER_TO_INT( g_object_get_data( G_OBJECT( button ), "gird" ) );

	if ( index == 1 )
	{
		if (  gtk_toggle_button_get_active( button ) )
			gird |= GTK_PLOT_GIRD_X;
		else
			gird &= ~GTK_PLOT_GIRD_X;
	}

	if ( index == 2 )
	{
		if (  gtk_toggle_button_get_active( button ) )
			gird |= GTK_PLOT_GIRD_Y;
		else
			gird &= ~GTK_PLOT_GIRD_Y;
	}

	gtk_plot_set_gird ( GTK_PLOT(user_data), gird );
}

int main (int argc, char *argv[])
{
 	GtkWidget *window;
 	GtkWidget *hbox, *vbox, *vbox_main;
 	GtkWidget *frame;
	GtkWidget *plot;
	GtkWidget *button;
	GtkWidget *combo;
	GArray *points;
	gdouble point[2], val;
	guint i;

	gtk_init (&argc, &argv);

	window = create_window ();

	hbox = gtk_hbox_new( FALSE, 6 );
	gtk_container_add( GTK_CONTAINER( window ), hbox );

	/* Tworzy ramke z napisem Wykres oraz ustawia jego rozmiary */
	frame = gtk_frame_new( "Wykres" );
	gtk_box_pack_start( GTK_BOX( hbox ), frame, TRUE, TRUE, 0 );


	plot = gtk_plot_new ();

	points = g_array_new ( FALSE, FALSE, sizeof(gdouble) );
	g_signal_connect( G_OBJECT( plot ), "button-press-event", G_CALLBACK( cb_press ), points );

	/*for ( i = 0; i < 20; i++ )
	{
		point[0] = i;
		point[1] = i+1;
		g_array_append_vals( points, (gconstpointer)point, 2 );
	}*/

  for (i=0; i<25; i++)
	{
		val=((gdouble)i-2)/15;
		point[0] = val*150;
		//g_array_append_val(x, val);
		val--;
		val*=sin(G_PI*val);
		point[1] = val*150;
		g_array_append_vals( points, (gconstpointer)point, 2 );
	}

/*	gtk_plot_add_data ( GTK_PLOT(plot), points );*/
	g_object_set ( plot, "data", points, NULL );
	gtk_container_add( GTK_CONTAINER (frame), plot );

	vbox_main = gtk_vbox_new( FALSE, 6 );
	gtk_box_pack_start( GTK_BOX( hbox ), vbox_main, FALSE, FALSE, 0 );

	frame = gtk_frame_new( "Interpolacja" );
	gtk_box_pack_start( GTK_BOX( vbox_main ), frame, FALSE, FALSE, 0 );

	vbox = gtk_vbox_new( TRUE, 6 );
  gtk_container_add( GTK_CONTAINER( frame ), vbox );

  button = gtk_check_button_new_with_label( "Punkty" );
  //gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), TRUE );
  g_object_set_data( G_OBJECT( button ), "type",
                  GINT_TO_POINTER( SPLINE_POINT ) );
	g_signal_connect( G_OBJECT( button ), "toggled", G_CALLBACK( cb_toggled ), plot );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );

	button = gtk_check_button_new_with_label( "Linie" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), TRUE );
	g_object_set_data( G_OBJECT( button ), "type",
                  GINT_TO_POINTER( SPLINE_LINE ) );
	g_signal_connect( G_OBJECT( button ), "toggled", G_CALLBACK( cb_toggled ), plot );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );

	button = gtk_check_button_new_with_label( "Zaokraglone" );
	g_object_set_data( G_OBJECT( button ), "type",
                  GINT_TO_POINTER( SPLINE_CATMULL_ROM ) );
	g_signal_connect( G_OBJECT( button ), "toggled", G_CALLBACK( cb_toggled ), plot );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );

	button = gtk_check_button_new_with_label( "Linie X" );
	gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( button ), TRUE );
	g_object_set_data( G_OBJECT( button ), "gird", GINT_TO_POINTER(1) );
	g_signal_connect( G_OBJECT( button ), "toggled", G_CALLBACK( cb_gird ), plot );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	gtk_plot_set_gird ( GTK_PLOT(plot), GTK_PLOT_GIRD_X );
	
	button = gtk_check_button_new_with_label( "Linie Y" );
	g_object_set_data( G_OBJECT( button ), "gird", GINT_TO_POINTER(2) );
	g_signal_connect( G_OBJECT( button ), "toggled", G_CALLBACK( cb_gird ), plot );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );
	
	frame = gtk_frame_new( "Ustawienia" );
  gtk_box_pack_start( GTK_BOX( vbox_main ), frame, FALSE, FALSE, 0 );

	vbox = gtk_vbox_new( TRUE, 6 );
  gtk_container_add( GTK_CONTAINER( frame ), vbox );
		
	button = gtk_spin_button_new_with_range( 1.0, 5, 0.1 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( button ), TRUE );
  gtk_spin_button_set_value( GTK_SPIN_BUTTON( button ), 1.0 );
  g_signal_connect( G_OBJECT( button ), "value-changed", G_CALLBACK( cb_value_line ), plot );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );

	button = gtk_spin_button_new_with_range( 0.1, 100.0, 0.1 );
	gtk_spin_button_set_numeric( GTK_SPIN_BUTTON( button ), TRUE );
  gtk_spin_button_set_value( GTK_SPIN_BUTTON( button ), 5.0 );
  g_signal_connect( G_OBJECT( button ), "value-changed", G_CALLBACK( cb_value_spline ), plot );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );

	combo = gtk_combo_box_text_new ();
	gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Top-Left" );
	gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Bottom-Left" );
	gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Top_Right" );
	gtk_combo_box_text_append_text ( GTK_COMBO_BOX_TEXT(combo), "Bottom-Right" );
	gtk_combo_box_set_active ( GTK_COMBO_BOX(combo), 0);
	g_signal_connect ( G_OBJECT(combo), "changed", G_CALLBACK(cb_combo), plot );
	gtk_box_pack_start( GTK_BOX( vbox ), combo, FALSE, FALSE, 0 );

	GdkColor color;

	color.red = 0;
	color.green = 0;
	color.blue = 65535;

	button = gtk_color_button_new_with_color (&color);
	g_signal_connect ( G_OBJECT(button), "color-set", G_CALLBACK(cb_color), plot );
	gtk_box_pack_start( GTK_BOX( vbox ), button, FALSE, FALSE, 0 );

	//gtk_plot_set_type ( GTK_PLOT(plot), GTK_PLOT_TYPE_LINE );

	
	//gtk_plot_set_reference_range ( plot, 20, 30 );
	g_object_set ( G_OBJECT(plot), "range-lower", 60, "range-upper", 30, "type", GTK_PLOT_LINE_TYPE_LINE | GTK_PLOT_LINE_TYPE_LCATMULL_ROM, NULL );

	guint x, y;
	g_object_get ( plot,  "range-lower", &x, "range-upper", &y, NULL );
	g_print ("X=%d   Y=%d\n", x, y);

	gtk_widget_show_all (window);

  gtk_main ();

	g_array_free ( points, FALSE );

  return 0;
}
