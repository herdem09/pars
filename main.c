#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include <json-c/json.h>

#define API_KEY ""
#define GEMINI_API_URL "https://generativelanguage.googleapis.com/v1/models/gemini-2.5-flash:generateContent?key="
#define MAX_RESPONSE_SIZE 65536

typedef struct {
    char *memory;
    size_t size;
} MemoryStruct;

typedef struct {
    GtkWidget *window;
    GtkWidget *text_view;
    GtkWidget *entry;
    GtkWidget *send_button;
    GtkTextBuffer *buffer;
    GtkTextTag *user_tag;
    GtkTextTag *pars_tag;
    GtkTextTag *command_tag;
    GtkTextTag *info_tag;
    GtkTextTag *error_tag;
    char current_command[1024];
} AppWidgets;

static AppWidgets *app = NULL;

static size_t write_callback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;
    
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(!ptr) return 0;
    
    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
    
    return realsize;
}

void add_message_to_buffer(const char *sender, const char *message, GtkTextTag *tag) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(app->buffer, &iter);
    gtk_text_buffer_insert(app->buffer, &iter, "\n", -1);

    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    char timestamp[16];
    strftime(timestamp, sizeof(timestamp), "%H:%M", t);

    if (strcmp(sender, "Pars") == 0) {
        // Logo ekle
        GError *error = NULL;
        GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale(
            "/home/herdem/Masaüstü/pars_1.0/images/logo_kucuk_sari.png", 16, 16, TRUE, &error);

        if (pixbuf) {
            GtkWidget *logo_img = gtk_image_new_from_pixbuf(pixbuf);
            GtkTextChildAnchor *anchor = gtk_text_buffer_create_child_anchor(app->buffer, &iter);
            gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(app->text_view), logo_img, anchor);
            gtk_widget_show(logo_img);
            g_object_unref(pixbuf);
            
            // Logo ile yazı arasında boşluk
            gtk_text_buffer_insert(app->buffer, &iter, " ", -1);
        }

        // Pars başlığı - aynı satırda
        char header[128];
        snprintf(header, sizeof(header), "Pars [%s]:", timestamp);
        gtk_text_buffer_insert_with_tags(app->buffer, &iter, header, -1, tag, NULL);
        gtk_text_buffer_insert(app->buffer, &iter, "\n", -1);
    }
    else if (strcmp(sender, "Kullanıcı") == 0) {
        char header[128];
        snprintf(header, sizeof(header), "👤 Sen [%s]:", timestamp);
        gtk_text_buffer_insert_with_tags(app->buffer, &iter, header, -1, tag, NULL);
        gtk_text_buffer_insert(app->buffer, &iter, "\n", -1);
    }
    else {
        char header[128];
        snprintf(header, sizeof(header), "⚙️ Sistem [%s]:", timestamp);
        gtk_text_buffer_insert_with_tags(app->buffer, &iter, header, -1, tag, NULL);
        gtk_text_buffer_insert(app->buffer, &iter, "\n", -1);
    }

    // Mesaj içeriği
    gtk_text_buffer_get_end_iter(app->buffer, &iter);
    gtk_text_buffer_insert(app->buffer, &iter, message, -1);
    gtk_text_buffer_insert(app->buffer, &iter, "\n", -1);

    // Otomatik kaydırma
    GtkTextMark *mark = gtk_text_buffer_get_insert(app->buffer);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(app->text_view), mark, 0.0, TRUE, 0.0, 1.0);
}

char* send_to_gemini(const char *user_message) {
    CURL *curl;
    CURLcode res;
    MemoryStruct chunk = {.memory = malloc(1), .size = 0};
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if(!curl) return strdup("API bağlantısı kurulamadı!");
    
    char url[512];
    snprintf(url, sizeof(url), "%s%s", GEMINI_API_URL, API_KEY);
    
    char system_prompt[] = "Sen Pardus Linux için bir komut asistanısın. Adın Pars. "
                          "Kullanıcının isteklerini analiz edip uygun Linux terminal komutlarını üret.\n\n"
                          "Yanıt formatın:\n"
                          "Açıklama: Kısa açıklama\n"
                          "```komut``` (3 backtick içinde)\n"
                          "Gerekirse güvenlik vb. uyarısı";
    
    json_object *jobj = json_object_new_object();
    json_object *contents = json_object_new_array();
    json_object *content = json_object_new_object();
    json_object *parts = json_object_new_array();
    json_object *part = json_object_new_object();
    
    char full_prompt[2048];
    snprintf(full_prompt, sizeof(full_prompt), "%s\n\nKullanıcı: %s", system_prompt, user_message);
    
    json_object_object_add(part, "text", json_object_new_string(full_prompt));
    json_object_array_add(parts, part);
    json_object_object_add(content, "parts", parts);
    json_object_array_add(contents, content);
    json_object_object_add(jobj, "contents", contents);
    
    const char *json_string = json_object_to_json_string(jobj);
    
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_string);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    res = curl_easy_perform(curl);
    
    char *result = NULL;
    if(res != CURLE_OK) {
        result = strdup("API isteği başarısız!");
    } else {
        json_object *response = json_tokener_parse(chunk.memory);
        if(response) {
            json_object *candidates, *candidate, *content_obj, *parts_obj, *part_obj, *text;
            
            if(json_object_object_get_ex(response, "candidates", &candidates) &&
               json_object_array_length(candidates) > 0) {
                
                candidate = json_object_array_get_idx(candidates, 0);
                if(json_object_object_get_ex(candidate, "content", &content_obj) &&
                   json_object_object_get_ex(content_obj, "parts", &parts_obj) &&
                   json_object_array_length(parts_obj) > 0) {
                    
                    part_obj = json_object_array_get_idx(parts_obj, 0);
                    if(json_object_object_get_ex(part_obj, "text", &text)) {
                        result = strdup(json_object_get_string(text));
                    }
                }
            }
            json_object_put(response);
        }
        
        if(!result) result = strdup("Yanıt alınamadı!");
    }
    
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
    curl_global_cleanup();
    free(chunk.memory);
    json_object_put(jobj);
    
    return result;
}

void execute_command(const char *command) {
    add_message_to_buffer("Sistem", "Terminal açılıyor...", app->info_tag);
    
    char script_path[256];
    snprintf(script_path, sizeof(script_path), "/tmp/pars_cmd_%d.sh", getpid());
    
    FILE *script = fopen(script_path, "w");
    if(!script) {
        add_message_to_buffer("Sistem", "Komut dosyası oluşturulamadı!", app->error_tag);
        return;
    }
    
    fprintf(script, "#!/bin/bash\n");
    fprintf(script, "echo '=== Pars Komut Çalıştırıcı ==='\n");
    fprintf(script, "echo 'Komut: %s'\n", command);
    fprintf(script, "echo ''\n");
    fprintf(script, "%s\n", command);
    fprintf(script, "EXIT_CODE=$?\n");
    fprintf(script, "echo ''\n");
    fprintf(script, "if [ $EXIT_CODE -eq 0 ]; then\n");
    fprintf(script, "    echo '✓ İşlem başarılı! Çıktılarla işiniz bittiğinde bu pencereyi kapatabilirsiniz.'\n");
    fprintf(script, "else\n");
    fprintf(script, "    echo '✗ Hata kodu: '$EXIT_CODE\n");
    fprintf(script, "fi\n");
    fprintf(script, "sleep 2\n");
    fprintf(script, "rm -f %s\n", script_path);
    fprintf(script, "exit $EXIT_CODE\n");
    fclose(script);
    
    chmod(script_path, 0755);
    
    char terminal_cmd[512];
    snprintf(terminal_cmd, sizeof(terminal_cmd), 
            "kgx -e 'bash %s' &", script_path);
    
    int result = system(terminal_cmd);
    
    if(result == 0) {
        add_message_to_buffer("Sistem", "Terminal başlatıldı.", app->info_tag);
    } else {
        add_message_to_buffer("Sistem", "Terminal başlatılamadı!", app->error_tag);
        unlink(script_path);
    }
}

void on_copy_clicked(GtkWidget *widget, gpointer data) {
    GtkClipboard *clipboard = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_set_text(clipboard, app->current_command, -1);
    add_message_to_buffer("Sistem", "Komut kopyalandı!", app->info_tag);
}

void on_execute_clicked(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog = gtk_message_dialog_new(GTK_WINDOW(app->window),
                                              GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_MESSAGE_QUESTION,
                                              GTK_BUTTONS_YES_NO,
                                              "Bu komutu çalıştır?\n\n%s",
                                              app->current_command);
    
    gint result = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    if(result == GTK_RESPONSE_YES) {
        execute_command(app->current_command);
    }
}

void* process_message_thread(void *data) {
    char *message = (char *)data;
    char *response = send_to_gemini(message);
    
    char *cmd_start = strstr(response, "```");
    char command[512] = {0};
    
    if(cmd_start) {
        cmd_start += 3;
        
        while(*cmd_start && (*cmd_start == ' ' || *cmd_start == '\n')) cmd_start++;
        if(strncmp(cmd_start, "bash", 4) == 0 || strncmp(cmd_start, "sh", 2) == 0) {
            while(*cmd_start && *cmd_start != '\n') cmd_start++;
            cmd_start++;
        }
        
        char *cmd_end = strstr(cmd_start, "```");
        if(cmd_end) {
            size_t len = cmd_end - cmd_start;
            if(len < sizeof(command)) {
                strncpy(command, cmd_start, len);
                command[len] = '\0';
                
                char *trim = command;
                while(*trim && (*trim == ' ' || *trim == '\n')) trim++;
                memmove(command, trim, strlen(trim) + 1);
                
                char *end = command + strlen(command) - 1;
                while(end > command && (*end == ' ' || *end == '\n' || *end == '\r')) *end-- = '\0';
            }
        }
    }
    
    add_message_to_buffer("Pars", response, app->pars_tag);
    
    if(strlen(command) > 0) {
        strncpy(app->current_command, command, sizeof(app->current_command) - 1);
        
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(app->buffer, &iter);
        
        GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
        GtkWidget *copy_btn = gtk_button_new_with_label("📋 Kopyala");
        GtkWidget *exec_btn = gtk_button_new_with_label("▶️ Çalıştır");
        
        g_signal_connect(copy_btn, "clicked", G_CALLBACK(on_copy_clicked), NULL);
        g_signal_connect(exec_btn, "clicked", G_CALLBACK(on_execute_clicked), NULL);
        
        gtk_box_pack_start(GTK_BOX(button_box), copy_btn, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(button_box), exec_btn, FALSE, FALSE, 0);
        
        GtkTextChildAnchor *anchor = gtk_text_buffer_create_child_anchor(app->buffer, &iter);
        gtk_text_view_add_child_at_anchor(GTK_TEXT_VIEW(app->text_view), button_box, anchor);
        gtk_widget_show_all(button_box);
    }
    
    free(response);
    free(message);
    
    gtk_widget_set_sensitive(app->send_button, TRUE);
    gtk_button_set_label(GTK_BUTTON(app->send_button), "Gönder ➤");
    
    return NULL;
}

void on_send_clicked(GtkWidget *widget, gpointer data) {
    const char *text = gtk_entry_get_text(GTK_ENTRY(app->entry));
    if(strlen(text) == 0) return;
    
    add_message_to_buffer("Kullanıcı", text, app->user_tag);
    
    gtk_widget_set_sensitive(app->send_button, FALSE);
    gtk_button_set_label(GTK_BUTTON(app->send_button), "⏳ İşleniyor...");
    
    char *msg_copy = strdup(text);
    gtk_entry_set_text(GTK_ENTRY(app->entry), "");
    
    g_thread_new("process_thread", process_message_thread, msg_copy);
}

void create_ui(GtkApplication *gtk_app) {
    app = g_malloc(sizeof(AppWidgets));
    
    app->window = gtk_application_window_new(gtk_app);
    gtk_window_set_title(GTK_WINDOW(app->window), "Pars Asistan");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 900, 700);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider,
        "GtkWindow { background-color: #1d1d1b; }"
        "GtkBox, GtkScrolledWindow { background-color: #1d1d1b; }"
        "GtkTextView { background-color: #1d1d1b; color: #ffcc00; }"
        "GtkEntry { background-color: #1d1d1b; color: #ffcc00; }"
        "GtkButton { background-color: #1d1d1b; color: #ffcc00; border: 1px solid #ffcc00; }"
        "GtkButton:hover { background-color: #ffcc00; color: #1d1d1b; }"
        , -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(app->window);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(app->window), main_box);

    GtkWidget *header = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_top(header, 15);
    gtk_widget_set_margin_bottom(header, 15);
    gtk_widget_set_margin_start(header, 20);

    GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_scale("/home/herdem/Masaüstü/pars_1.0/images/logo.png", 48, 48, TRUE, NULL);
    GtkWidget *logo = gtk_image_new_from_pixbuf(pixbuf);
    g_object_unref(pixbuf);

    GtkWidget *title_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
    GtkWidget *title = gtk_label_new("Pars Asistan");
    GtkWidget *subtitle = gtk_label_new("Pardus Komut Rehberi");

    PangoAttrList *attrs = pango_attr_list_new();
    pango_attr_list_insert(attrs, pango_attr_size_new(18 * PANGO_SCALE));
    pango_attr_list_insert(attrs, pango_attr_weight_new(PANGO_WEIGHT_BOLD));
    gtk_label_set_attributes(GTK_LABEL(title), attrs);

    gtk_box_pack_start(GTK_BOX(title_box), title, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(title_box), subtitle, FALSE, FALSE, 0);

    gtk_box_pack_start(GTK_BOX(header), logo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(header), title_box, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), header, FALSE, FALSE, 0);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_margin_start(scrolled, 10);
    gtk_widget_set_margin_end(scrolled, 10);
    gtk_widget_set_margin_bottom(scrolled, 10);

    app->text_view = gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app->text_view), GTK_WRAP_WORD);
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app->text_view), FALSE);
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(app->text_view), 10);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(app->text_view), 10);

    app->buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(app->text_view));

    app->user_tag = gtk_text_buffer_create_tag(app->buffer, "user", 
                                               "foreground", "#ffcc00",
                                               "weight", PANGO_WEIGHT_BOLD, NULL);
    app->pars_tag = gtk_text_buffer_create_tag(app->buffer, "pars",
                                               "foreground", "#ffcc00",
                                               "weight", PANGO_WEIGHT_BOLD, NULL);
    app->command_tag = gtk_text_buffer_create_tag(app->buffer, "command",
                                                  "foreground", "#1d1d1b",
                                                  "background", "#ffcc00",
                                                  "family", "monospace", NULL);
    app->info_tag = gtk_text_buffer_create_tag(app->buffer, "info",
                                               "foreground", "#ffcc00", NULL);
    app->error_tag = gtk_text_buffer_create_tag(app->buffer, "error",
                                                "foreground", "#ffcc00", NULL);

    gtk_container_add(GTK_CONTAINER(scrolled), app->text_view);
    gtk_box_pack_start(GTK_BOX(main_box), scrolled, TRUE, TRUE, 0);

    GtkWidget *input_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_margin_start(input_box, 10);
    gtk_widget_set_margin_end(input_box, 10);
    gtk_widget_set_margin_bottom(input_box, 10);

    app->entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app->entry), "Komut isteğinizi yazın...");
    g_signal_connect(app->entry, "activate", G_CALLBACK(on_send_clicked), NULL);

    app->send_button = gtk_button_new_with_label("Gönder ➤");
    gtk_widget_set_size_request(app->send_button, 100, -1);
    g_signal_connect(app->send_button, "clicked", G_CALLBACK(on_send_clicked), NULL);

    gtk_box_pack_start(GTK_BOX(input_box), app->entry, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(input_box), app->send_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(main_box), input_box, FALSE, FALSE, 0);

    add_message_to_buffer("Pars", "Merhaba! Size nasıl yardımcı olabilirim?", app->pars_tag);

    gtk_widget_show_all(app->window);
}

void activate(GtkApplication *gtk_app, gpointer user_data) {
    create_ui(gtk_app);
}

int main(int argc, char **argv) {
    GtkApplication *gtk_app = gtk_application_new("tr.org.pardus.pars", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(gtk_app, "activate", G_CALLBACK(activate), NULL);
    
    int status = g_application_run(G_APPLICATION(gtk_app), argc, argv);
    g_object_unref(gtk_app);
    
    return status;
}
