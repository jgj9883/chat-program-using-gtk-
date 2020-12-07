#include <gtk/gtk.h>
typedef struct _Data Data;
struct _Data {
  GtkWidget *window;
  GtkWidget *PortEntry; /*포트번호가 들어가는 텍스트 엔트리 위젯 포인터 저장*/
  GtkWidget *ConBtn; /*접속 버튼 위젯 포인터 저장*/
  GtkWidget *SendBtn; /*전송 버튼 위젯 포인터 저장*/
  GtkWidget *ChatTextView; /*채팅이 나타날 텍스트 뷰 위젯 포인터 저장*/
  GtkWidget *msgEntry; /*메시지가 적힐 텍스트 엔트리 위젯 포인터 저장*/

};

G_MODULE_EXPORT void quit (GtkWidget *window,gpointer data){
	gtk_main_quit ();
}

G_MODULE_EXPORT void on_ConBtn_clicked(GtkButton *button, Data *data){
 GtkTextBuffer* buffer =gtk_text_view_get_buffer(GTK_TEXT_VIEW(data->ChatTextView));
 GtkTextIter end;
/*해당 텍스트뷰의 텍스트뷰버퍼 받아오기*/
 gtk_text_buffer_get_end_iter(buffer,&end);
/*텍스트뷰 버퍼의 마지막에 해당하는 위치 가져오기.*/
 gtk_text_buffer_insert(buffer,&end,"8080 접속", -1);
/*마지막에 해당하는 위치에 계속 뿌리기.*/
 
}

G_MODULE_EXPORT void on_SendBtn_clicked(GtkButton *button, Data *data){
 
	gtk_text_buffer_insert_at_cursor (buf, "\n", -1);
	// text view 버퍼 가져오기
	GtkWidget *buf = gtk_text_view_get_buffer (GTK_TEXT_VIEW (data->msgEntry));
	
	gchar *message = gtk_entry_get_text (GTK_ENTRY (data->ChatTextView));	

	// 버퍼초기화
	//gtk_text_buffer_set_text (buf, "\n", -1);

	
	gtk_text_buffer_insert_at_cursor (buf, message, -1);
	gtk_text_buffer_insert_at_cursor (buf, "\n", -1);
 
}

int main (int argc, char *argv[]){
  GtkBuilder *builder;
  GError *error;
  Data *data;
  gtk_init (&argc, &argv);
  /* 빌더 생성 및 UI 파일 열기 */
  builder = gtk_builder_new ();
  if (!gtk_builder_add_from_file(builder,"gtk_client.glade",NULL)) {
/*	"g_print ("UI 파일을 읽을 때 오류 발생!\n");"*/
/*	"g_print ("메시지: %s\n", error->message);*/
	/*g_free (error);*/
	/*return (1);*/
	}

  data = g_slice_new (Data); /*메모리 버퍼 초기화*/

  data->window = GTK_WIDGET(gtk_builder_get_object (builder, "GtkWindow"));
  data->PortEntry = GTK_WIDGET(gtk_builder_get_object (builder, "PortEntry"));
  data->ConBtn = GTK_WIDGET(gtk_builder_get_object (builder, "ConBtn"));
  data->SendBtn = GTK_WIDGET(gtk_builder_get_object (builder, "SendBtn"));
  data->msgEntry = GTK_WIDGET(gtk_builder_get_object (builder, "msgEntry"));
  data->ChatTextView = GTK_WIDGET(gtk_builder_get_object (builder, "ChatTextView"));

  gtk_builder_connect_signals (builder, data);
  g_object_unref (G_OBJECT (builder));
  gtk_widget_show_all (data->window);
  gtk_main ();
  g_slice_free (Data, data);
return (0);
} 
