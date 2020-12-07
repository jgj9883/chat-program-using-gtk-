#include <gtk/gtk.h>
typedef struct _Data Data;
struct _Data 
{
	GtkWidget *window;
	GtkWidget *button;
	GtkWidget *label;
	GtkWidget *textView;
	GtkWidget *textBuffer;
	GtkWidget *textField;
};

// 메세지 전송 버튼 클릭 시
G_MODULE_EXPORT void on_btnSend_clicked (GtkWidget *button, Data *data)
{
	gtk_text_buffer_insert_at_cursor (buf, "\n", -1);
	// text view 버퍼 가져오기
	GtkWidget *buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data->textView));
	
	gchar *message = gtk_entry_get_text (GTK_ENTRY (data->textField));	

	// 버퍼초기화
	//gtk_text_buffer_set_text (buf, "\n", -1);

	
	gtk_text_buffer_insert_at_cursor (buf, message, -1);
	gtk_text_buffer_insert_at_cursor (buf, "\n", -1);

	gtk_label_set_text (GTK_LABEL (data->label), "send 누름");
	
}

G_MODULE_EXPORT void on_btnOpen_clicked (GtkButton *button, Data *data)
{
	
	gtk_label_set_text (GTK_LABEL (data->label), "방이 개설되었습니다!");
	GtkWidget *buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data->textView));
	gtk_text_buffer_set_text (buf, "\n", -1);
	gtk_text_buffer_insert_at_cursor (buf, "포트번호 : 8080  ", -1);
	gtk_text_buffer_set_text (buf, "\n", -1);
}

int main (int argc, char *argv[])
{
	GtkBuilder *builder;
	GError *error;
	Data *data;
	gtk_init (&argc, &argv);
	
	/* 빌더 생성 및 UI 파일 열기 */
	builder = gtk_builder_new ();
	
	if ( !gtk_builder_add_from_file (builder,"gtk_server.glade",NULL)) 
	{
		//g_print ("UI 파일을 읽을 때 오류 발생!\n");
		//g_print ("메시지: %s\n", error->message);
		//g_free (error);
		//return (1);
	}

	data = g_slice_new (Data);
	data->window = GTK_WIDGET(gtk_builder_get_object (builder, "window"));
	data->button = GTK_WIDGET(gtk_builder_get_object (builder, "btnSend"));
	data->button = GTK_WIDGET(gtk_builder_get_object (builder, "btnOpen"));
	data->label = GTK_WIDGET(gtk_builder_get_object (builder, "label"));
	data->textView = GTK_WIDGET(gtk_builder_get_object (builder, "textViewLog"));
	data->textBuffer = GTK_WIDGET(gtk_builder_get_object (builder, "textBuffer"));
	data->textField = GTK_WIDGET(gtk_builder_get_object (builder, "textFieldMessage"));

	gtk_builder_connect_signals (builder, data);
	g_object_unref (G_OBJECT (builder));
	gtk_widget_show_all (data->window);
	gtk_main ();
	g_slice_free (Data, data);
	return (0);
} 
