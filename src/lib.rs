use colored::{Color, Colorize};

const LOGO: &str = r#"
              .oooooo.                          
Yb           d8P'  `Y8b                         
 `Yb        888          oooo  oooo  oo.ooooo.  
   `Yb      888          `888  `888   888' `88b 
   .dP      888           888   888   888   888 
 .dP        `88b    ooo   888   888   888   888 
dP           `Y8bood8P'   `V88V"V8P'  888bod8P' 
                                      888       
                                     o888o      
"#;

pub fn main_page() {
    const GRAY: Color = Color::TrueColor {
        r: 128,
        g: 128,
        b: 128,
    };

    println!("{}", LOGO.bright_cyan());
    println!(
        "{} What's the {}? {}\n",
        ">>>".color(GRAY),
        "Cup".bold(),
        "<<<".color(GRAY)
    );
    println!(
        "### Cup is a {} tool for C++ {} and {}!",
        "Cargo-like".bold().underline(),
        "Project Building".bold().underline(),
        "Package Management".bold().underline()
    );
    println!("### Now, it will be rewritten in Rust!");
    println!("### Let C++ and Rust progress together!\n");
}
