use async_file_hasher::calculate_md5_hash;
use async_file_hasher::calculate_md5_hash_sync;
use std::env;
use std::time::Instant;

#[tokio::main]
async fn main() -> Result<(), Box<dyn std::error::Error + Send + Sync>> {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        eprintln!("Usage: {} <file1> [file2] [file3] ...", args[0]);
        std::process::exit(1);
    }

    let file_names = &args[1..];

    println!("\nProcessing {} files using Rust async:", file_names.len());
    let start_async = Instant::now();

    let mut handles = Vec::new();

    for file_name in file_names {
        let file_name = file_name.clone();
        let handle = tokio::spawn(async move {
            match calculate_md5_hash(&file_name).await {
                Ok(hash) => {
                    println!("{file_name}: {hash}");
                    Ok(())
                }
                Err(e) => {
                    eprintln!("Error processing {file_name}: {e}");
                    Err(e)
                }
            }
        });
        handles.push(handle);
    }

    for handle in handles {
        let _ = handle.await?;
    }

    let async_duration = start_async.elapsed();
    println!("Rust async processing time: {async_duration:?}");

    println!("\nProcessing {} files using Rust:", file_names.len());
    let start_sync = Instant::now();

    for file_name in file_names {
        match calculate_md5_hash_sync(file_name) {
            Ok(hash) => {
                println!("{file_name}: {hash}");
            }
            Err(e) => {
                eprintln!("Error processing {file_name}: {e}");
            }
        }
    }

    let sync_duration = start_sync.elapsed();
    println!("Rust processing time: {sync_duration:?}");
    Ok(())
}
