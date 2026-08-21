
const galleryImages = document.querySelectorAll(".gallery-item img");

const lightbox = document.getElementById("lightbox");

const lightboxImg = document.getElementById("lightbox-img");

const closeBtn = document.querySelector(".close-lightbox");
let lastFocusedElement;

const closeLightbox = () => {
    lightbox.classList.remove("active");
    lightbox.setAttribute("aria-hidden", "true");
    document.body.classList.remove("lightbox-open");

    if (lastFocusedElement) {
        lastFocusedElement.focus();
    }
};

galleryImages.forEach(image => {

    const galleryItem = image.parentElement;

    image.addEventListener("click", () => {

        lastFocusedElement = image;
        lightbox.classList.add("active");
        lightbox.setAttribute("aria-hidden", "false");
        document.body.classList.add("lightbox-open");
        lightboxImg.src = image.src;
        lightboxImg.alt = image.alt;
        closeBtn.focus();

    });

    galleryItem.addEventListener("keydown", event => {
        if (event.key === "Enter" || event.key === " ") {
            event.preventDefault();
            image.click();
        }
    });

});

closeBtn.addEventListener("click", () => {
    closeLightbox();
});

lightbox.addEventListener("click", e => {

    if(e.target===lightbox){
        closeLightbox();
    }

});

document.addEventListener("keydown",e=>{

    if(e.key==="Escape"){
        closeLightbox();
    }

});



// ================= MOBILE MENU =================

const menuToggle = document.getElementById("menu-toggle");
const menuClose = document.getElementById("menu-close");
const navLinks = document.getElementById("nav-links");

const setMenuState = isOpen => {
    navLinks.classList.toggle("active", isOpen);
    menuToggle.classList.toggle("active", isOpen);
    menuClose.classList.toggle("active", isOpen);
    menuToggle.setAttribute("aria-expanded", String(isOpen));
};

menuToggle.addEventListener("click", () => setMenuState(true));
menuClose.addEventListener("click", () => setMenuState(false));

window.addEventListener("resize", () => {
    if (window.innerWidth > 768) {
        setMenuState(false);
    }
}, { passive: true });

[menuToggle, menuClose].forEach(button => {
    button.addEventListener("keydown", event => {
        if (event.key === "Enter" || event.key === " ") {
            event.preventDefault();
            button === menuToggle ? setMenuState(true) : setMenuState(false);
        }
    });
});

document.querySelectorAll(".nav-links a").forEach(link => {
    link.addEventListener("click", () => {
        setMenuState(false);
    });
});

// ================= COUNTERS =================

const counters = document.querySelectorAll(".counter");

const counterObserver = new IntersectionObserver((entries)=>{

    entries.forEach(entry=>{

        if(entry.isIntersecting){

            const counter = entry.target;

            const target = +counter.dataset.target;

            let count = 0;

            const speed = target / 80;

            const update = ()=>{

                count += speed;

                if(count < target){

                    counter.innerText = Math.ceil(count);

                    requestAnimationFrame(update);

                }else{

                    counter.innerText = target + "+";

                }

            };

            update();

            counterObserver.unobserve(counter);

        }

    });

});

counters.forEach(counter=>counterObserver.observe(counter));

// ================= LOADER =================

const loader = document.getElementById("loader");

window.addEventListener("load", () => {
    window.setTimeout(() => loader.classList.add("hide"), 450);
}, { once: true });

// ================= SCROLL STATE =================

const progressBar = document.getElementById("progress-bar");
const sections = document.querySelectorAll("section[id]");
const navLinksItems = document.querySelectorAll('nav a[href^="#"]');
let scrollFrame;

const updateScrollState = () => {
    const scrollHeight = document.documentElement.scrollHeight - window.innerHeight;
    const progress = scrollHeight > 0 ? (window.scrollY / scrollHeight) * 100 : 0;
    progressBar.style.width = `${progress}%`;

    let current = "";
    sections.forEach(section => {
        if (window.scrollY >= section.offsetTop - 120) {
            current = section.id;
        }
    });

    navLinksItems.forEach(link => {
        link.classList.toggle("active", link.getAttribute("href") === `#${current}`);
    });

    scrollFrame = null;
};

window.addEventListener("scroll", () => {
    if (!scrollFrame) {
        scrollFrame = window.requestAnimationFrame(updateScrollState);
    }
}, { passive: true });

updateScrollState();

// ================= SCROLL ANIMATION =================

const hiddenElements = document.querySelectorAll(
"section"
);

const observer = new IntersectionObserver((entries)=>{

entries.forEach(entry=>{

if(entry.isIntersecting){

entry.target.classList.add("show");

}

});

});

hiddenElements.forEach(el=>{

el.classList.add("hidden");

observer.observe(el);

});

// ================= ACTIVE NAV ================

