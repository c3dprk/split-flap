def lam_mats(mat1, mat2, lam):
    return [[lam(c, d) for (c, d) in zip(a, b)] for (a, b) in zip(mat1, mat2)]

def comp_mats(mat1, mat2):
    return lam_mats(mat1, mat2, lambda a, b: a == b)

def diff_mod_mats(mat1, mat2, mod):
    return lam_mats(mat1, mat2, lambda a, b: (a-b)%mod)

def diff_min_mats(mat1, mat2, min):
    return lam_mats(mat1, mat2, lambda a, b: max(a-b, min))
