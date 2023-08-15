// Prevents additional console window on Windows in release, DO NOT REMOVE!!
#![cfg_attr(not(debug_assertions), windows_subsystem = "windows")]

use tauri::api::process::{Command, CommandEvent};

use simple_websockets::{Event, Responder};
use std::collections::HashMap;

async fn start_server() {
  // listen for WebSockets on port 8080:
  let event_hub = simple_websockets::launch(8080)
    .expect("failed to listen on port 8080");
  // map between client ids and the client's `Responder`:
  let mut clients: HashMap<u64, Responder> = HashMap::new();

  loop {
    match event_hub.poll_event() {
        Event::Connect(client_id, responder) => {
            println!("A client connected with id #{}", client_id);
            // add their Responder to our `clients` map:
            clients.insert(client_id, responder);
        },
        Event::Disconnect(client_id) => {
            println!("Client #{} disconnected.", client_id);
            // remove the disconnected client from the clients map:
            clients.remove(&client_id);
        },
        Event::Message(client_id, message) => {
            println!("Received a message from client #{}: {:?}", client_id, message);
            for (key, responder) in &clients {
              // echo the message back:
              responder.send(message.clone());
            }
        },
    }
  }
}

fn main() {
    let (mut rx, mut child) = Command::new_sidecar("main")
        .expect("failed to create `main` binary command")
        .spawn()
        .expect("Failed to spawn sidecar");

    tauri::Builder::default()
        .setup(move |app| {
            tauri::async_runtime::spawn(start_server());

            Ok(())
        })
        .run(tauri::generate_context!())
        .expect("error while running tauri application");
}
